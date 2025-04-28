#include "TypeToStringFormatter.h"
#include <cassert>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <format>

namespace TimeFormatHelper{
std::string const formatDateTime = "{:%Y-%m-%d %H:%M:%S}";
std::string const formatDateTimeNoSec = "{:%Y-%m-%d %H:%M}";
std::string const formatDate = "{:%Y-%m-%d}";
std::string const formatTime = "{:%H:%M:%S}";
std::map<DataInfo::Type, std::string> const dateTimeTypeToFormatString = {
    {DataInfo::DateTime, formatDateTime},
    {DataInfo::DateTimeNoSec, formatDateTimeNoSec},
    {DataInfo::Date, formatDate},
    {DataInfo::Time, formatTime}
};

std::string chronoSysSecToString(const std::chrono::sys_seconds dateTimeInSecs, DataInfo::Type nType)
{
    string formattedString;
     try {
        switch (nType) {
            case DataInfo::DateTime:
                return std::format("{:%Y-%m-%d %H:%M:%S}", dateTimeInSecs);
            case DataInfo::DateTimeNoSec:
                return std::format("{:%Y-%m-%d %H:%M}", dateTimeInSecs);
            case DataInfo::Date:
                return std::format("{:%Y-%m-%d}", dateTimeInSecs);
            case DataInfo::Time:
                return std::format("{:%H:%M:%S}", dateTimeInSecs);
            default:
                throw std::invalid_argument(FORMATER_ERR_WRONG_DATE_TYME_TYPE);
        }
     } catch (const std::format_error&) {
         throw std::invalid_argument(FORMATER_ERR_CHRONO_FORMAT);
     }
    return formattedString;
}

std::chrono::sys_seconds stringTochronoSysSec(const string & formattedDateTime, DataInfo::Type nType)
{
    std::chrono::sys_seconds dateTimeInSecs;
    std::istringstream ss(formattedDateTime);

    switch (nType) {
        case DataInfo::DateTime:
            ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", dateTimeInSecs);
            break;
        case DataInfo::DateTimeNoSec:
            ss >> std::chrono::parse("%Y-%m-%d %H:%M", dateTimeInSecs);
            break;
        case DataInfo::Date:
            ss >> std::chrono::parse("%Y-%m-%d", dateTimeInSecs);
            break;
        case DataInfo::Time:
            ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", dateTimeInSecs);
            break;
        default:
            throw std::invalid_argument(FORMATER_ERR_WRONG_DATE_TYME_TYPE);
    }

    if (ss.fail()) {
        throw std::invalid_argument(FORMATER_ERR_STRING_FORMAT);
    }

    return dateTimeInSecs;
}

std::chrono::sys_seconds chronoNow()
{
    return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
}

} // namespace

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
        throw std::invalid_argument(FORMATER_ERR_WRONG_TYPE);
    }
    dataList.push_back(info);
}

void TypeToStringFormatter::AddDataInfo(const char * paramName, std::chrono::sys_seconds paramValue, DataInfo::Type nType)
{
    DataInfo info;
    info.param = paramName;
    info.value = TimeFormatHelper::chronoSysSecToString(paramValue, nType);
    info.type = nType;

    dataList.push_back(info);
}

void TypeToStringFormatter::AddDataInfo(const char *paramName, const char *paramValue, DataInfo::Type nType)
{
    DataInfo info;
    info.param = paramName;
    info.type = nType;
    // Check validity of provided value
    std::chrono::sys_seconds sysSecs = TimeFormatHelper::stringTochronoSysSec(paramValue, nType);
    info.value = TimeFormatHelper::chronoSysSecToString(sysSecs, nType);

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
        throw std::invalid_argument(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME);
    }
    return it->value;
}

DataInfo TypeToStringFormatter::dataInfo(const char *paramName)
{
    auto it = std::find_if(dataList.begin(), dataList.end(),
                           [&](const DataInfo & info) { return info.param == paramName; });

    if(it == dataList.end()) {
        throw std::invalid_argument(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME);
    }
    return *it;
}

std::chrono::sys_seconds TypeToStringFormatter::toTime(const char * paramName)
{
    auto it = std::find_if(dataList.begin(), dataList.end(),
                           [&](const DataInfo & info) { return info.param == paramName; });

    if(it == dataList.end()) {
        throw std::invalid_argument(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME);
    }

    static std::set<DataInfo::Type> trueDateTimes({DataInfo::DateTime, DataInfo::DateTimeNoSec, DataInfo::Date, DataInfo::Time});
    if(!trueDateTimes.contains(it->type)) {
        throw std::invalid_argument(FORMATER_ERR_WRONG_DATE_TYME_TYPE);
    }

    string infoValue = it->value;
    if(it->type == DataInfo::Time) {
        infoValue = string("1-1-1 ") + infoValue;
    }

    return TimeFormatHelper::stringTochronoSysSec(infoValue, it->type);
}

/*
    std::string input = "1211-10-11 10:11:12";

    // 1. Parse input into std::chrono::local_time
    // Parse into a sys_seconds (UTC)
    std::istringstream ss(input);
    std::chrono::sys_seconds tp;
    ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", tp);

    if (ss.fail()) {
        std::cerr << "Parsing failed\n";
        return 1;
    }

    // Directly format sys_time without converting to time_t
    std::cout << "Parsed UTC time: " << std::format("{:%Y-%m-%d %H:%M:%S}", tp) << '\n';
*/
