#include "env.h"
#include <mutex>
#include <string>

static std::once_flag initEnvMap;
static Environment::EnvironmentMap GlobalEnvMap; 

static std::pair<std::string, std::string> StringToKV(const std::string &s) {
    std::size_t pos = s.find('=');
    if (pos == std::string::npos) {
        return std::pair<std::string,std::string>(s,"");
    }
    return std::pair<std::string,std::string>(
        s.substr(0, pos),
        s.substr(pos + 1)
    );
}

#ifdef _WIN32
static void initGlobalEnvMap() {
    auto free = [](LPTCH p) {
        FreeEnvironmentStrings(p);
    };

    auto env_block = std::unique_ptr<TCHAR, decltype(free)>(
                GetEnvironmentStrings(), free);

    for (LPTCH i = env_block.get(); *i != TCHAR('\0'); ++i) {
        std::string key;
        std::string value;

        for (; *i != TCHAR('='); ++i) {
            key += *i;
        }

        for (++i; *i != TCHAR('\0'); ++i) {
            value += *i;
        }

        GlobalEnvMap[key] = value;
    }
}
#elif __linux__
extern char **environ;
static void initGlobalEnvMap() {
    for (char **env = environ; *env; ++env) {
        auto kv = StringToKV(*env);
        GlobalEnvMap.insert(kv);
    }
}
#endif

Environment::EnvironmentMap Environment::GetEnv(const Environment::Config &cfg) {
    std::call_once(initEnvMap, initGlobalEnvMap);

    EnvironmentMap envMap;
    for (auto kv : GlobalEnvMap) {
        if (cfg.prefix != "" && kv.first.compare(0, cfg.prefix.length(), cfg.prefix) != 0) {
            continue;
        }
        envMap.insert(kv);
    }
    return envMap;
}
