#ifndef _READER_H_
#define _READER_H_

#include "config.h"
#include <string>

class Reader {
public:
    typedef Config::json Values; 
    typedef std::string SourceData;
    enum ParseCode {
        OK,
        Failed,
        StreamError
    };
public:
    virtual ParseCode Parse(const SourceData &data, Values &values) = 0;
    virtual ParseCode Parse(std::istream &is, Values &value) = 0;
    virtual bool SupportStream() const = 0;
    virtual std::string Type() const = 0;
};

#endif