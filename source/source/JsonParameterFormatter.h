#ifndef JSONPARAMETERFORMATTER_H
#define JSONPARAMETERFORMATTER_H

#include "TypeToStringFormatter.h"

namespace {
const char * FORMATER_ERR_WRONG_JSON = "Wrong format JSON as parameter!";
}
class JsonParameterFormatter : public TypeToStringFormatter
{
public:
    JsonParameterFormatter();

    string toJsonString();
    static map<string, string>  fromJsonString(const string & jsonString);

    void AddDataInfo(const char * paramName, const char * paramValue);
    void AddDataInfo(const char * paramName, int paramValue);
    void AddDataInfo(const char * paramName, int64_t paramValue);
    void AddDataInfo(const char * paramName, double paramValue);
    void AddDataInfo(const char * paramName, bool paramValue);

    void AddDataInfo(const char * paramName,  std::chrono::milliseconds paramValue, DataInfo::Type nType);
    void AddDataInfo(const char * paramName,  const char * paramValue, DataInfo::Type nType);
};

#endif // JSONPARAMETERFORMATTER_H
