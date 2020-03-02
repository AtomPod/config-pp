#include <iostream>
#include "../config.h"
#include "../env.h"

struct Database {
    std::string filename;
};

void from_json(const Config::json &j, Database &d) {
    d.filename = j.at("filename").get<std::string>();
}

int main(int, char**) {
    setenv("CPP_ATOM_GOGO", "beta", 0);

    Config conf;
    conf.AddConfigPath(".");
    conf.SetConfigType("json");
    conf.SetConfigName("config");
    conf.SetEnvPrefix("CPP_");
    conf.AutomaticEnv();

    conf.SetDefault({"atom", "delta"}, 15);

    if (conf.ReadInConfig() != Config::OK) {
        std::cerr << "failed to load config\n";
        return 0;
    }

    auto val = conf.Get<Database>({"databases", "leveldb"});
    if (val.hasValue) {
        std::cout << val.value.filename << '\n';
    }

    auto atomVal = conf.Get<std::string>({"atom", "gogo"});
    if (atomVal.hasValue) {
        std::cout << atomVal.value << '\n';
    }

    conf.WriteConfig(std::cout) << '\n';
    return 0;
}
