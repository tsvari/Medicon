#include "TypeToStringFormatter.h"
#include <cassert>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace TimeFormatHelper{
std::string const formatDateTime = "%Y-%m-%d %H:%M:%S";
std::string const formatDateTimeNoSec = "%Y-%m-%d %H:%M";
std::string const formatDate = "%Y-%m-%d";
std::string const formatTime = "%H:%M:%S";
std::map<DataInfo::Type, std::string> const dateTimeTypeToFormatString = {
    {DataInfo::DateTime, formatDateTime},
    {DataInfo::DateTimeNoSec, formatDateTimeNoSec},
    {DataInfo::Date, formatDate},
    {DataInfo::Time, formatTime}
};

std::string timeToString(const time_t dateTime, DataInfo::Type nType)
{
    struct tm timeStruct;

#if defined(_WIN32) && defined(_MSC_VER)
    localtime_s(&timeStruct, &dateTime);
#else
    timeStruct = localtime(&dateTime);
#endif

    char buffer[32];
    if(dateTimeTypeToFormatString.contains(nType)) {
        strftime (buffer, sizeof(buffer), dateTimeTypeToFormatString.at(nType).c_str(), &timeStruct);
    }
    return std::string(buffer);
}
}

void TypeToStringFormatter::AddDataInfo(const char * paramName, FormatterDataType & paramValue)
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
        // unknown type
        assert(true);
    }
    dataList.push_back(info);
}

void TypeToStringFormatter::AddDataInfo(const char * paramName, const time_t paramValue, DataInfo::Type nType)
{
    DataInfo info;
    info.param = paramName;
    info.value = TimeFormatHelper::timeToString(paramValue, nType);
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

string TypeToStringFormatter::value(const char * paramName)
{
    auto it = std::find_if(dataList.begin(), dataList.end(),
                           [&](const DataInfo & info) { return info.param == paramName; });

    if(it == dataList.end()) {
        assert(true);
    }
    return it->value;
}

DataInfo TypeToStringFormatter::dataInfo(const char *paramName)
{
    auto it = std::find_if(dataList.begin(), dataList.end(),
                           [&](const DataInfo & info) { return info.param == paramName; });

    if(it == dataList.end()) {
        assert(true);
    }
    return *it;
}

time_t TypeToStringFormatter::toTime(const char * paramName)
{
    auto it = std::find_if(dataList.begin(), dataList.end(),
                           [&](const DataInfo & info) { return info.param == paramName; });

    if(it == dataList.end()) {
        assert(true);
    }

    if(!TimeFormatHelper::dateTimeTypeToFormatString.contains(it->type)) {
        return -1;
    }

    time_t datTime;
    struct tm tm{};
    std::istringstream dateTimeValue(it->value);

    dateTimeValue >> std::get_time(&tm, TimeFormatHelper::dateTimeTypeToFormatString.at(it->type).c_str());

    if(it->type == DataInfo::Time) {
        // No Date by default so put starting Date
        tm.tm_year = 71;
        tm.tm_mon = 0;
        tm.tm_mday = 1;
    }

    return std::mktime(&tm);
}
