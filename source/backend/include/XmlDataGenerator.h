#pragma once

#include <vector>
#include <string>
#include "global_def.h"

using std::string;
using std::vector;


class XmlDataGenerator
{
public:
    XmlDataGenerator(void){}
    virtual ~XmlDataGenerator(void){}

    void AddDbInfo(const char* tgName, const char* tgValue,   unsigned int nType = GlobalType::String);
    void AddDbInfo(const char* tgName, const string& tgValue,   unsigned int nType = GlobalType::String);
    void AddDbInfo(const char* tgName, const bool tgValue,     unsigned int nType = GlobalType::Int);
    void AddDbInfo(const char* tgName, const double tgValue,   unsigned int nType = GlobalType::Double);
    void AddDbInfo(const char* tgName, const int tgValue,      unsigned int nType = GlobalType::Int);
    void AddDbInfo(const char* tgName, const time_t tgValue,   unsigned int nType);

    inline const vector<DataInfo>& GetData() const {return db_xml_info_arr_;}
    inline string&		GetError(){ return last_error_msg_; }

#ifndef TIDA_OS_ANDROID
    bool LoadXmlData(const string& xml);
#endif

    string GenerateXmlData() const;
    string GetValue(const char* tgName);

protected:
    vector<DataInfo> db_xml_info_arr_;
    string		last_error_msg_;
};

