#include "TypeToStringFormatter.h"
#include <cassert>
#include <sstream>
#include <format>
#include <random>

namespace TimeFormatHelper{
std::string chronoSysSecToString(const std::chrono::milliseconds dateTimeInSecs, DataInfo::Type nType)
{
    std::chrono::sys_seconds dts = floor<std::chrono::seconds>(std::chrono::sys_time<std::chrono::milliseconds>{dateTimeInSecs});
    string formattedString;
     try {
        switch (nType) {
            case DataInfo::DateTime:
                return std::format("{:%Y-%m-%d %H:%M:%S}", dts);
            case DataInfo::DateTimeNoSec:
                return std::format("{:%Y-%m-%d %H:%M}", dts);
            case DataInfo::Date:
                return std::format("{:%Y-%m-%d}", dts);
            case DataInfo::Time:
                return std::format("{:%H:%M:%S}", dts);
            default:
                throw std::invalid_argument(FORMATER_ERR_WRONG_DATE_TYME_TYPE);
        }
     } catch (const std::format_error&) {
         throw std::invalid_argument(FORMATER_ERR_CHRONO_FORMAT);
     }
    return formattedString;
}

std::string chronoSysSecToString(int64_t dateTimeInSecs, DataInfo::Type nType)
{
    //std::chrono::seconds duration(dateTimeInSecs);
    std::chrono::milliseconds timePoint{dateTimeInSecs};
    return chronoSysSecToString(timePoint, nType);
}

std::chrono::milliseconds stringTochronoSysSec(const string & formattedDateTime, DataInfo::Type nType)
{
    std::chrono::sys_time<std::chrono::milliseconds> dateTimeInSecs;
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

    return dateTimeInSecs.time_since_epoch();
}

std::chrono::milliseconds chronoNow()
{
    return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());;
}

std::string generateUniqueString() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 127);

    std::vector<uint8_t> randomBytes(64);
    for (int i = 0; i < 64; ++i) {
        randomBytes[i] = distrib(gen);
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : randomBytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

} // namespace

void TypeToStringFormatter::AddDataInfo(const char * paramName, FormatterDataType & paramValue)
{
    DataInfo info;
    info.param = paramName;

    if (int * ptr = std::get_if<int>(&paramValue)) {
        info.value = std::to_string(*ptr);
        info.type = DataInfo::Int;
    } else if (int64_t * ptr = std::get_if<int64_t>(&paramValue)) {
        info.value = std::to_string(*ptr);
        info.type = DataInfo::Int64;
    }else if (double * ptr = std::get_if<double>(&paramValue)) {
        info.value = std::to_string(*ptr);
        info.type = DataInfo::Double;
    } else if (string * ptr = std::get_if<string>(&paramValue)) {
        info.value = *ptr;
        info.type = DataInfo::String;
    } else if (bool * ptr = std::get_if<bool>(&paramValue)) {
        info.value = std::to_string((*ptr == true)?1:0);
        info.type = DataInfo::Bool;
    } else {
        // unknown type
        throw std::invalid_argument(FORMATER_ERR_WRONG_TYPE);
    }
    dataList.push_back(info);
}

void TypeToStringFormatter::AddDataInfo(const char * paramName, std::chrono::milliseconds paramValue, DataInfo::Type nType)
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
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(paramValue, nType);
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

std::chrono::milliseconds TypeToStringFormatter::toTime(const char * paramName)
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
        // There is no Time alone without a Date, so write something like 1-1-1 to avoid being thrown DataInfo::Time type
        infoValue = string("1-1-1 ") + infoValue;
    }

    return TimeFormatHelper::stringTochronoSysSec(infoValue, it->type);
}

