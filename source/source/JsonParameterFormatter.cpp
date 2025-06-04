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
    json doc = json::parse(jsonString);
    try {
        doc = json::parse(jsonString);
    } catch (json::parse_error & ex) {
        throw std::invalid_argument(FORMATER_ERR_WRONG_XML);
    }
    return doc.get<map<string, string>>();
}

