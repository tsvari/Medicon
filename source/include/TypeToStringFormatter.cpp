#include "TypeToStringFormatter.h"
#include <cassert>
#include <ctime>

TypeToStringFormatter::TypeToStringFormatter()
{

}

void TypeToStringFormatter::AddDataInfo(const char *paramName, DataType & paramValue)
{
    DataInfo info;
    info.param = paramName;

    if (int * ptr = std::get_if<int>(&paramValue)) {
        info.value = std::to_string(*ptr);
        info.type = DataInfo::Int;
    } else if (double * ptr = std::get_if<double>(&paramValue)) {
        info.value = std::to_string(*ptr);
        info.type = DataInfo::Double;
    } else if (string * ptr = std::get_if<string>(&paramValue)) {
        info.value = *ptr;
        info.type = DataInfo::String;
    } else if (bool * ptr = std::get_if<bool>(&paramValue)) {
        info.value = std::to_string((*ptr == true)?1:0);
        info.type = DataInfo::Int;
    } else {
        // No known type
        assert(true);
    }
    dataList.push_back(info);
}

void TypeToStringFormatter::AddDataInfo(const char * paramName, const time_t paramValue, DataInfo::Type nType)
{
    DataInfo info;
    info.param = paramName;

    struct tm timeStruct;

#if defined(_WIN32) && defined(_MSC_VER)
    localtime_s(&timeStruct, &paramValue);
#else
    timeStruct = localtime(&paramValue);
#endif

    char buffer[32];

    switch(nType) {
    case DataInfo::DateTime:
        strftime (buffer, 32, "%Y-%m-%d %H:%M:%S", &timeStruct);
        break;
    case DataInfo::DateTimeNoSec:
        strftime (buffer, 32, "%Y-%m-%d %H:%M", &timeStruct);
        break;
    case DataInfo::Date:
        strftime (buffer, 32, "%Y-%m-%d", &timeStruct);
        break;
    case DataInfo::Time:
        strftime (buffer, 32, "%H:%M:%S", &timeStruct);
        break;
    default:
        info.value = "";
    }

    std::string s(buffer);
    info.value.assign(s.begin(), s.end());
    info.type = nType;

    dataList.push_back(info);
}

map<string, string> TypeToStringFormatter::formattedParamValueList() const
{
    map<string, string> paramValue;
    for(auto const & info: dataList) {
        paramValue.insert(std::make_pair(info.param, info.value));
    }
    return paramValue;
}
