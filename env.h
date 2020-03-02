#ifndef _ENV_H_
#define _ENV_H_
#include <map>
#include <string>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

class Environment {
public:
#ifdef _WIN32
    typedef std::basic_string<TCHAR> tstring;
#elif __linux__
    typedef std::string tstring;
#endif
    typedef std::map<tstring, tstring> EnvironmentMap;

    struct Config {
        //指定环境变量的前缀字符串
        std::string prefix;
    };
public:
    //GetEnv 获取环境变量数据，cfg为对应的获取信息
    static EnvironmentMap GetEnv(const Config &cfg = Config());
};

#endif