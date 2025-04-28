#ifndef TYPETOSTRINGFORMATTER_H
#define TYPETOSTRINGFORMATTER_H

#include "include_util.h"
#include <chrono>

namespace {
const char * FORMATER_ERR_STRING_FORMAT = "Format should be {:%Y-%m-%d %H:%M:%S}!";
const char * FORMATER_ERR_CHRONO_FORMAT = "The provided chronoseconds should be parsed using {:%Y-%m-%d %H:%M:%S}!";
const char * FORMATER_ERR_WRONG_DATE_TYME_TYPE = "The specified argument is not a date or time type!";
const char * FORMATER_ERR_WRONG_KEY_PARAMETER_NAME = "Wrong name for provided parameter!";
const char * FORMATER_ERR_WRONG_TYPE = "The system does not support this type!";
} // namespace
namespace TimeFormatHelper {
extern std::string const formatDateTime;
extern std::string const formatDateTimeNoSec;
extern std::string const formatDate;
extern std::string const formatTime;
extern std::map<DataInfo::Type, std::string> const dateTimeTypeToFormatString;

std::string chronoSysSecToString(const std::chrono::sys_seconds dateTimeInSecs, DataInfo::Type nType);
std::chrono::sys_seconds stringTochronoSysSec(const std::string & formattedDateTime, DataInfo::Type nType);
std::chrono::sys_seconds chronoNow();
} // namespace

typedef variant<int, double, string, bool> FormatterDataType;

class TypeToStringFormatter
{
public:
    TypeToStringFormatter(){}

    void AddDataInfo(const char * paramName, FormatterDataType & paramValue);
    void AddDataInfo(const char * paramName,  std::chrono::sys_seconds paramValue, DataInfo::Type nType);
    void AddDataInfo(const char * paramName,  const char * paramValue, DataInfo::Type nType);
    map<string, string> formattedParamValueList() const;

    string value(const char * paramName);
    DataInfo dataInfo(const char * paramName);
    // To reuse for local output
    std::chrono::sys_seconds toTime(const char * paramName);

    vector<DataInfo> & dataInfoList(){return dataList;}

private:
    vector<DataInfo> dataList;

};

#endif // TYPETOSTRINGFORMATTER_H
