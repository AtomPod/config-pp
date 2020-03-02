#include "yaml.h"
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

static bool yamlToJson(YAML::Node node, Reader::Values &target) {
    for (auto beg = node.begin(); beg != node.end(); ++beg) {
        std::string key = beg->first.as<std::string>();
        YAML::Node subnode = beg->second;

        switch (subnode.Type()) {
        case YAML::NodeType::Scalar:
            try {
                target[key] = subnode.as<double>();
            } catch (...) {
                target[key] = subnode.as<std::string>();
            }
            break;
        case YAML::NodeType::Sequence:
            target[key] = nlohmann::json::array();
            yamlToJson(subnode, target[key]);
            break;
        case YAML::NodeType::Map:
            target[key] = nlohmann::json::object();
            yamlToJson(subnode, target[key]);
            break;
        case YAML::NodeType::Null:
            target[key] = nullptr;
            break;
        default:
            break;
        }
    }
}

YamlReader::YamlReader() {

}

YamlReader::~YamlReader() {

}

Reader::ParseCode YamlReader::Parse(const Reader::SourceData &data, Reader::Values &values) {
    YAML::Node config = YAML::Load(data.c_str());
    if (config.IsNull()) {
        return Reader::Failed;
    }

    values = nlohmann::json::object();
    yamlToJson(config, values);
    return OK;
}

Reader::ParseCode YamlReader::Parse(std::istream &is, Reader::Values &value) {
    return Failed;
}

bool YamlReader::SupportStream() const {
    return false;
}

std::string YamlReader::Type() const {
    return "yaml";
}