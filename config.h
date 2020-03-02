#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <nlohmann/json.hpp>
#include <initializer_list>
#include <list>
#include <mutex>

class Reader;
//Config 配置类，用于加载和获取配置数据
class Config {
public:
    template<typename T>
    struct Value {
        T value;
        bool hasValue;
    };

    enum ErrorCode {
        OK = 0,
        FileNotFound,
        LoadFileFailed,
        ParseFileFailed,
        IsNotObject,
        StreamError,
        TypeUnsupported
    };

    typedef std::list<std::string> ConfigPathList;
public:
    typedef nlohmann::json json;
public:
    Config();
    virtual ~Config();
public:
    //Get 获取对应的配置信息，其中keys参数为对应的路径
    //例如{"a","b","c"}则获取对应的路径a/b/c，而非每一个都是一个值
    //若存在则获取成功，返回值中hasValue为true，否则hasValue为false
    template<typename T>
    Value<T> Get(const std::initializer_list<std::string> &keys);

    //SetDefault 设置对应keys路径的默认值为value
    //注：该函数会无视源数据进行覆盖，若路径上面对应的其中一个值
    //不为对象，那么无法覆盖，则返回false,若成功，则返回true
    template<typename T>
    bool SetDefault(const std::initializer_list<std::string> &keys, const T &value);

    //AddConfigPath 添加配置文件搜索的路径path
    void AddConfigPath(const std::string &path);
    //SetConfigName 设置配置文件的文件名，该文件名不包含扩展后缀
    void SetConfigName(const std::string &name);
    //SetConfigType 设置配置文件的类型（即扩展后缀）
    void SetConfigType(const std::string &type);
    //SetEnvPrefix  设置解析环境变量的前缀，如CONFIG_
    //那么将匹配CONFIG_ID，CONFIG_NAME，CONFIG_DATABASE等
    //若解析完成，所有值将会转换为小写，如CONFIG_ID，将会作为值id
    void SetEnvPrefix(const std::string &envPrefix);
    //AutomaticEnv 开启解析环境变量，若不调用该函数，那么
    //即使设置SetEnvPrefix，也不会解析环境变量
    void AutomaticEnv();

    const ConfigPathList &ConfigPaths() const { return m_configPaths; }
    const std::string ConfigFile() const { return m_configFile; }
    const std::string ConfigName() const { return m_configName; }
    const std::string ConfigType() const { return m_configType; }
    const std::string EnvPrefix() const  { return m_envPrefix;  }
    bool IsAutomaticEnv() const { return m_automaticEnv; }

    //ReadInConfig 从添加的配置解析信息中，解析数据，调用该函数后，将会真正解析文件和环境变量数据，
    //若解析成功，返回OK，否则返回错误码
    ErrorCode ReadInConfig();
    //MergeConfigJson 将外部的数据合并到配置数据中
    //若成功，返回OK,否则返回错误码
    ErrorCode MergeConfigJson(json j);
    //MergeConfigJson 从输入流中读取数据，并合并到配置数据中
    //若成功，返回OK,否则返回错误码
    ErrorCode MergeConfigJson(std::istream &is, const std::string &type = "json");
    //WriteConfig 将配置信息格式化写入到输出流中
    //返回值为输入参数os
    std::ostream &WriteConfig(std::ostream &os) const;
    //SetDefault 静态函数，用于设置全局的配置对象
    static void SetDefault(Config *c);
    //Default 静态函数，返回全局的配置对象
    static Config *Default();
    //RegisterReader 注册读取器
    static bool RegisterReader(Reader *r);
    //UnregisterReader 注销读取器
    static bool UnregisterReader(Reader *r);
    //GetReader 获取对应类型的读取器
    static Reader *GetReader(const std::string &name);
protected:
    ErrorCode tryLoadFile(const std::string &filepath, json &config);
    ErrorCode mergeEnvConfig();
    ErrorCode mergeFileConfig();
    ErrorCode mergeToConfig(json &target, json src);
    json *keysObject(const std::initializer_list<std::string> &keys);
private:
    bool                    m_automaticEnv;
    json                    m_config;
    ConfigPathList          m_configPaths;
    std::string             m_configName;
    std::string             m_configType;
    std::string             m_envPrefix;
    std::string             m_configFile;
    mutable std::mutex      m_mutex;
};

template<typename T>
Config::Value<T> Config::Get(const std::initializer_list<std::string> &path) {
    std::unique_lock<std::mutex> locker(m_mutex);
    
    Value<T> value;
    value.hasValue = false;
    json *conf = &m_config;

    for (auto p : path) {
        auto it = conf->find(p);
        if (it == conf->end()) {
            return value;
        }
        conf = &(*it);
    }

    try {
        value.value = conf->get<T>();
        value.hasValue = true;
    } catch (std::exception const &) {
        value.hasValue = false;
    }
    return value;
}

template<typename T>
bool Config::SetDefault(const std::initializer_list<std::string> &keys, const T &value) {
    std::unique_lock<std::mutex> locker(m_mutex);
    
    auto object = keysObject(keys);
    if (object != nullptr) {
        (*object) = value;
        return true;
    }
    return false;
}

#endif