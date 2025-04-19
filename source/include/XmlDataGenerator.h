#pragma once

#include "include_util.h"

class XmlDataGenerator
{
public:
    XmlDataGenerator(void){}
    virtual ~XmlDataGenerator(void){}

    void AddDbInfo(const char * tgName, const char * tgValue, unsigned int nType = DataInfo::String);
    void AddDbInfo(const char * tgName, const string & tgValue, unsigned int nType = DataInfo::String);
    void AddDbInfo(const char * tgName, const bool tgValue, unsigned int nType = DataInfo::Int);
    void AddDbInfo(const char * tgName, const double tgValue, unsigned int nType = DataInfo::Double);
    void AddDbInfo(const char * tgName, const int tgValue, unsigned int nType = DataInfo::Int);
    void AddDbInfo(const char * tgName, const time_t tgValue, unsigned int nType);

    bool LoadXmlData(const string & xml);
    string GenerateXmlData() const;
    string value(const char * tgName);
    inline const vector<DataInfo> & xmlData() const {return db_xml_info_arr_;}
    inline string & error(){ return last_error_msg_; }

private:
    vector<DataInfo> db_xml_info_arr_;
    string last_error_msg_;
};

