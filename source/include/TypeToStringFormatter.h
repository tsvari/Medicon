#ifndef TYPETOSTRINGFORMATTER_H
#define TYPETOSTRINGFORMATTER_H

#include "include_util.h"

namespace TimeFormatHelper{
extern std::string const formatDateTime;
extern std::string const formatDateTimeNoSec;
extern std::string const formatDate;
extern std::string const formatTime;
extern std::map<DataInfo::Type, std::string> const dateTimeTypeToFormatString;

std::string timeToString(const time_t dateTime, DataInfo::Type nType);
}

typedef variant<int, double, string, bool> FormatterDataType;

class TypeToStringFormatter
{
public:
    TypeToStringFormatter(){}

    void AddDataInfo(const char * paramName, FormatterDataType & paramValue);
    void AddDataInfo(const char * paramName, const time_t paramValue, DataInfo::Type nType);
    map<string, string> formattedParamValueList() const;

    string value(const char * paramName);
    DataInfo dataInfo(const char * paramName);
    // To reuse for local output
    time_t toTime(const char * paramName);

private:
    vector<DataInfo> dataList;

};

#endif // TYPETOSTRINGFORMATTER_H
