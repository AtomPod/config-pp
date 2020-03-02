#include "config.h"
#include "env.h"
#include <fstream>
#include <atomic>

std::string pathJoin(const std::initializer_list<std::string> &pathList, const std::string &ext = "") {
    std::string paths;
    for (auto path : pathList) {
        if (path.back() == '/') {
            path.pop_back();
        }
        if (!paths.empty()) {
            paths += "/";
        }
        paths += path;
    }

    if (ext != "") {
        if (ext[0] != '.') {
            paths += '.';
        }
        paths += ext;
    }
    return paths;
}

std::list<std::string> stringSplit(const std::string &s, const std::string &delim) {
    std::size_t endIndex = s.find_first_of(delim);
    std::size_t startIndex = 0;

    if (endIndex == std::string::npos) {
        return std::list<std::string>{s};
    }

    std::list<std::string> lists;
    do {
        lists.push_back(s.substr(startIndex, endIndex - startIndex));
        startIndex = endIndex + 1;
        endIndex = s.find_first_of(delim, startIndex);
    } while (endIndex != std::string::npos);

    lists.push_back(s.substr(startIndex));
    return lists;
}

void stringToLower(std::string &s) {
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            s[i] = s[i] - 'A' + 'a';
        }
    }
}

Config::Config() {
    m_configName = "config";
    m_automaticEnv = false;
}

Config::~Config() {
    
}

void Config::AddConfigPath(const std::string &path) {
    m_configPaths.push_back(path);
}

void Config::SetConfigName(const std::string &name) {
    m_configName = name;
}

void Config::SetConfigType(const std::string &type) {
    m_configType = type;
}

Config::ErrorCode Config::ReadInConfig() {
    ErrorCode code = OK;

    if (m_automaticEnv) {
        code = mergeEnvConfig();
        if (code != OK) {
            return code;
        }
    }

    return mergeFileConfig();
}

Config::ErrorCode Config::MergeConfigJson(Config::json j) {
    if (!j.is_object()) {
        return IsNotObject;
    }
    return mergeToConfig(m_config, j);
}

Config::ErrorCode Config::MergeConfigJson(std::istream &is) {
    json streamJson;
    if (!(is >> streamJson)) {
        return StreamError;
    }
    return MergeConfigJson(streamJson);
}

std::ostream &Config::WriteConfig(std::ostream &os) const {
    os << m_config;
    return os;
}

Config::ErrorCode Config::mergeToConfig(Config::json &target, Config::json src) {
    //实现参考(RFC 7386)https://tools.ietf.org/html/rfc7386
    //若源数据不是一个object，即不是一个对象，那么不进行合并
    //若target本身不是一个object，那么重新作为一个object
    //遍历所有src中的key/value值，若src中key对应的value
    //为null，那么说明需要删除数据，那么将target中
    //对应的值删除，否则合并
    if (src.is_object()) {
        if (!target.is_object()) {
            target = json::object();
        }

        for (auto beg = src.begin(); beg != src.end(); ++beg) {
            std::string key = beg.key();
            if (!beg->is_null()) {
                if (beg->is_object()) {
                    mergeToConfig(target[key], beg.value());
                } else {
                    target[key] = beg.value();
                }
            } else {
                target.erase(key);
            }
        }
        return OK;
    }
    return OK;
}

Config::ErrorCode Config::mergeEnvConfig() {
    static const std::string spaceChars = " \r\n\f\v\b";

    Environment::Config config;
    config.prefix = m_envPrefix;

    json envJsonConfig;
    Environment::EnvironmentMap envMap = Environment::GetEnv(config);
    
    //遍历所有环境变量，并解析环境变量对，将环境变量转换为一个json数据
    //由于可能出现这种情况，CONFIG_DATABASE__HOST,那么解析时，会
    //解析成{CONFIG,DATABASE,,HOST},那么可是因为误打一个_，
    //作为修正，那么将跳过空白符对应的位置，即上述会转换为
    //{CONFIG,DATABASE,HOST}的路径进行保存，若出现冲突
    //即其中一个key可能不为object，那么此时无法继续储存，
    //那么不覆盖
    for (auto kv : envMap) {
        
        std::string keyRMPrefix = kv.first.substr(m_envPrefix.length());
        std::list<std::string> keyLists = stringSplit(keyRMPrefix, "_");

        json *root = &envJsonConfig;
        size_t keys = keyLists.size();
        auto beg = keyLists.begin();
        for (size_t k = 0; k < keys - 1; ++k) {
            std::string key = *beg++;
            stringToLower(key);
            if (key.find_first_not_of(spaceChars) == std::string::npos) {
                continue;
            }

            auto it = root->find(key);
            if (it != root->end() && !it->is_object()) {
                break;
            }
            root = &(*root)[key];
        }

        std::string key = *beg++;
        stringToLower(key);
        (*root)[key] = kv.second;
    }
    return MergeConfigJson(envJsonConfig);
}

Config::ErrorCode Config::mergeFileConfig() {
    ErrorCode code;
    json inJsonConfig;

    //查找所有路径中，对应的配置信文件，并调用
    //tryLoadFile从文件中读取，若读取失败，那么查找下一个位置
    //直到所有路径解析完成，或者其中一个读取完成则返回
    for (auto path : m_configPaths) {
        std::string filepath = pathJoin({path, m_configName},  m_configType);
        code = tryLoadFile(filepath, inJsonConfig);
        if (code == OK) {
            m_configFile = filepath;
            break;
        }
    }

    if (code != OK) {
        return code;
    }

    return MergeConfigJson(inJsonConfig);
}

Config::ErrorCode Config::tryLoadFile(const std::string &filepath, json &config) {
    std::ifstream ifs(filepath);
    if (!ifs) {
        return FileNotFound;
    }

    //获取读取的文件长度，用于申请内存使用
    ifs.seekg(0,std::ios::end);
    std::streampos length =  ifs.tellg();
    ifs.seekg(0,std::ios::beg);

    std::string fileContent(length, '\0');
    if (!ifs.read(&fileContent[0], length)) {
        return LoadFileFailed;
    }

    if (ifs.gcount() != length) {
        return LoadFileFailed;
    }   

    //尝试解析json数据，若解析失败，那么返回ParseFileFailed，
    try {
        config = json::parse(fileContent);
    } catch (std::exception const &e) {
        return ParseFileFailed;
    }
    return OK;
}

//keysObject 根据keys路径加载查询对应路径的对象指针
//若路径不存在或错误，那么返回nullptr
Config::json *Config::keysObject(const std::initializer_list<std::string> &keys) {
    json *object = &m_config;
    for (auto key : keys) {
       if (!object->is_object()) {
           return nullptr;
       }
       object = &(*object)[key];
    }

    if (&m_config == object) {
        return nullptr;
    }
    return object;
}

void Config::SetEnvPrefix(const std::string &envPrefix) {
    m_envPrefix = envPrefix;
}

void Config::AutomaticEnv() {
    m_automaticEnv = true;
}

static std::atomic<Config*> defaultConfig(nullptr);

void Config::SetDefault(Config *c) {
    defaultConfig.store(c);
}

Config *Config::Default() {
    return defaultConfig.load();
}