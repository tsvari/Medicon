#include "JsonParameterFormatter.h"

#include <nlohmann/json.hpp>

// Good example for objects https://github.com/nlohmann/json/issues/1281

JsonParameterFormatter::JsonParameterFormatter() : TypeToStringFormatter() {}

using json = nlohmann::json;

string JsonParameterFormatter::toJsonString()
{
    return json(formattedParamValueList()).dump();
}

map<string, string> JsonParameterFormatter::fromJsonString(const string & jsonString)
{
    json doc;
    try {
        doc = json::parse(jsonString);
    } catch (json::parse_error & ex) {
        throw std::invalid_argument(FORMATER_ERR_WRONG_XML);
    }
    return doc.get<map<string, string>>();
}

void JsonParameterFormatter::AddDataInfo(const char * paramName, const char * paramValue)
{
    FormatterDataType data(paramValue);
    TypeToStringFormatter::AddDataInfo(paramName, data);
}

void JsonParameterFormatter::AddDataInfo(const char * paramName, int paramValue)
{
    FormatterDataType data(paramValue);
    TypeToStringFormatter::AddDataInfo(paramName, data);
}

void JsonParameterFormatter::AddDataInfo(const char *paramName, int64_t paramValue)
{
    FormatterDataType data(paramValue);
    TypeToStringFormatter::AddDataInfo(paramName, data);
}

void JsonParameterFormatter::AddDataInfo(const char * paramName, double paramValue)
{
    FormatterDataType data(paramValue);
    TypeToStringFormatter::AddDataInfo(paramName, data);
}

void JsonParameterFormatter::AddDataInfo(const char * paramName, bool paramValue)
{
    FormatterDataType data(paramValue);
    TypeToStringFormatter::AddDataInfo(paramName, data);
}

void JsonParameterFormatter::AddDataInfo(const char *paramName, std::chrono::sys_seconds paramValue, DataInfo::Type nType)
{
    TypeToStringFormatter::AddDataInfo(paramName, paramValue, nType);
}

void JsonParameterFormatter::AddDataInfo(const char *paramName, const char * paramValue, DataInfo::Type nType)
{
    TypeToStringFormatter::AddDataInfo(paramName, paramValue, nType);
}

