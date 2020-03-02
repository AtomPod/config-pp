# config-pp
基于nlohmann/json的C++配置库,该库参考golang库的spf13/viper实现，再次感谢。

## 安装
    git clone https://github.com/phantom-atom/config-pp
    cmake 
    make 

## Config-PP
config-pp是一个C++配置库，该库使用nlohmann/json库作为储存数据，所有转换为json信息的数据，均可作为该库的数据源。包含以下功能：
    * 设置默认值
    * 读取JSON文件、JSON数据流
    * 从环境变量中读取指定数据（指定前缀）
    * 合并多个配置信息
    * 导出配置信息

### 设置默认配置数据
支持设置默认的配置数据，将键值对路径设置对应的默认值，若该路径上面已经某个值不是对象，那么将设置失败。

例子： 以下将路径database/host的值设置为localhost:5432
    Config config;
    config.SetDefault({"database", "host"}, "localhost:5432");

### 读取配置文件
目前仅支持JSON文件，通过设置配置搜索目录以及文件信息，将会自动搜索匹配的文件数据，多个文件也只会加载第一个遇到的文件，当加载失败，则会往下继续搜索。

例子：
    Config config;
    config.AddConfigPath(".");  //添加当前路径到搜索路径
    config.AddConfigPath("./config"); //添加当前路径的config目录到搜索路径
    config.SetConfigType("json"); //设置配置文件后缀为json
    config.SetConfigName("config"); //设置配置文件名（不包含后缀）
    if (config.ReadInConfig() != Config::OK) {
        std::cerr << "Failed to read configuare" << std::endl;
    }

### 读取环境变量
支持读取环境变量，同时支持指定的环境变量前缀，环境变量的名称将使用下划线(_)进行切分，即DATABASE_HOST，那么将会以{"database", "host"}作为keys，而且将会把所有字母转换为小写字母

例子：
    Config config;
    conf.SetEnvPrefix("CPP_"); //设置环境变量前缀为CPP_
    conf.AutomaticEnv(); //开启自动解析环境变量
    if (config.ReadInConfig() != Config::OK) {
        std::cerr << "Failed to read configuare" << std::endl;
    }

### 预计后续
    * 多配置文件格式
    * 配置信息修改的监听
    * 支持外部源信息
    * 远程源信息
    * argv参数解析
    * 配置子树