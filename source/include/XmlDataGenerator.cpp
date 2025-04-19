#include "XmlDataGenerator.h"
#include <time.h>
#include <cassert>

#include "Markup.h"

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const char * tgValue, unsigned int nType)
{
    DataInfo info;
    info.param = tgName;
    info.value = tgValue;
    info.type = nType;

    db_xml_info_arr_.push_back(info);
}

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const string & tgValue, unsigned int nType)
{
    DataInfo info;
    info.param = tgName;
    info.value = tgValue;
    info.type = nType;

    db_xml_info_arr_.push_back(info);
}

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const double tgValue, unsigned int nType)
{
    DataInfo info;
    info.param = tgName;
    info.value = std::to_string(tgValue);
    info.type = nType;

    db_xml_info_arr_.push_back(info);
}

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const bool tgValue, unsigned int nType)
{
    int nVal = 0;
    if(tgValue) nVal = 1;
    AddDbInfo(tgName, nVal, nType);
}

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const int tgValue, unsigned int nType)
{
    DataInfo info;
    info.param = tgName;
    info.value = std::to_string(tgValue);
    info.type = nType;

    db_xml_info_arr_.push_back(info);
}

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char * tgName, const time_t tgValue, unsigned int nType)
{
    DataInfo info;
    info.param = tgName;

    time_t dVal = tgValue;

    if(dVal<=0) {
        info.value = "";
        info.type = nType;
        db_xml_info_arr_.push_back(info);
        return;
    }

    struct tm * ptm2 = NULL;

#if defined(_WIN32) && defined(_MSC_VER)
    struct tm ptm;
    localtime_s(&ptm, &dVal);
    ptm2 = &ptm;
#else
    ptm2 = localtime(&dVal);
#endif

    char buffer[32];

    switch(nType) {
    case DataInfo::DateTime:
        strftime (buffer, 32, "%Y-%m-%d %H:%M:%S", ptm2);
        break;
    case DataInfo::DateTimeNoSec:
        strftime (buffer, 32, "%Y-%m-%d %H:%M", ptm2);
        break;
    case DataInfo::Date:
        strftime (buffer, 32, "%Y-%m-%d", ptm2);
        break;
    case DataInfo::Time:
        strftime (buffer, 32, "%H:%M:%S", ptm2);
        break;
    default:
        info.value = "";
    }

    std::string s(buffer);
    info.value.assign(s.begin(), s.end());

    info.type = nType;

    db_xml_info_arr_.push_back(info);
}


///////////////////////////////////////////////////////////////
//
//
//
string XmlDataGenerator::value(const char * tgName)
{
    for(auto const & info: db_xml_info_arr_) {
        if(info.param == tgName)
            return info.value;
    }
    assert(true);
    return "";
}

///////////////////////////////////////////////////////////////
//
//
//
string XmlDataGenerator::GenerateXmlData() const
{
    if(db_xml_info_arr_.size() <=0 )
        return string("");

    CMarkup doc;
    doc.AddElem("DATA");
    doc.IntoElem();

    for(size_t i=0; i<db_xml_info_arr_.size(); i++)
    {
        DataInfo info = db_xml_info_arr_[i];

        doc.AddElem("Param");
        doc.IntoElem();
        doc.AddElem("Name", info.param);
        doc.AddElem("Value", info.value);
        doc.AddElem("Type", info.type);
        doc.OutOfElem();
    }

    doc.OutOfElem();

    return doc.GetDoc();
}

///////////////////////////////////////////////////////////////
//
//
//
bool XmlDataGenerator::LoadXmlData(const string & xml)
{
    CMarkup doc;
    if (!doc.SetDoc(xml)) {
        last_error_msg_ = doc.GetError();
        return false;
    }

    doc.IntoElem(); // into DATA
    while (doc.FindChildElem("Param"))
    {
        doc.IntoElem();
        string paramName, paramValue, paramType;

        if (doc.FindChildElem("Name")) {
            paramName = doc.GetChildData();
        }
        if (doc.FindChildElem("Value")) {
            paramValue = doc.GetChildData();
        }
        if (doc.FindChildElem("Type")) {
            paramType = doc.GetChildData();
        }

        AddDbInfo(paramName.c_str(), paramValue.c_str(), std::stoi(paramType));

        doc.OutOfElem();
    }

    return true;
}

