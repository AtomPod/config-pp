#ifndef _READER_YAML_H_
#define _READER_YAML_H_
#include "../reader.h"

class YamlReader : public Reader {
public:
    YamlReader();
    ~YamlReader();
public:
    virtual ParseCode Parse(const SourceData &data, Values &values);
    virtual ParseCode Parse(std::istream &is, Values &value);
    virtual bool SupportStream() const;
    virtual std::string Type() const;
};

#endif