#ifndef TYPETOSTRINGFORMATTER_H
#define TYPETOSTRINGFORMATTER_H

#include "include_util.h"

typedef variant<int, double, string, bool> DataType;

class TypeToStringFormatter
{
public:
    TypeToStringFormatter();

    void AddDataInfo(const char * paramName, DataType & paramValue);
    void AddDataInfo(const char * paramName, const time_t paramValue, DataInfo::Type nType);
    map<string, string> formattedParamValueList() const;

private:
    vector<DataInfo> dataList;

};

#endif // TYPETOSTRINGFORMATTER_H
