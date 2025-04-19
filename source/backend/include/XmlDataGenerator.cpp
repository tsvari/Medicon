#include "XmlDataGenerator.h"
#include <time.h>


#ifndef TIDA_OS_ANDROID
    #include "Markup.h"
#else
    #include "QtXml/QtXml"
//    #include "QtXml/QDomDocument"
//    #include "QtXml/QDomElement"
#endif

namespace patch2
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}
///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const char* tgValue, unsigned int nType)
    {
    DataInfo info;
    info._tag_name = tgName;
    info._tag_value = tgValue;
    info._tag_type = nType;

    db_xml_info_arr_.push_back(info);
    }

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const string& tgValue, unsigned int nType)
    {
    DataInfo info;
    info._tag_name = tgName;
    info._tag_value = tgValue;
    info._tag_type = nType;

    db_xml_info_arr_.push_back(info);
    }
///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const double tgValue, unsigned int nType)
    {
    DataInfo info;
    info._tag_name = tgName;
    info._tag_value = patch2::to_string(tgValue);
    info._tag_type = nType;

    db_xml_info_arr_.push_back(info);
    }

///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const bool tgValue, unsigned int nType)
    {
    int nVal = 0;
    if(tgValue) nVal = 1;
    AddDbInfo(tgName, nVal, nType);
    }
///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const int tgValue, unsigned int nType)
    {
    DataInfo info;
    info._tag_name = tgName;
    info._tag_value = patch2::to_string(tgValue);
    info._tag_type = nType;

    db_xml_info_arr_.push_back(info);
    }
///////////////////////////////////////////////////////////////
//
//
//
void XmlDataGenerator::AddDbInfo(const char* tgName, const time_t tgValue, unsigned int nType)
	{
	DataInfo info;
	info._tag_name = tgName;

	time_t dVal = tgValue;

    if(dVal<=0)
        {
        info._tag_value = "";
        info._tag_type = nType;
        db_xml_info_arr_.push_back(info);
        return;
        }

    struct tm *ptm2=NULL;

    #if defined(_WIN32) && defined(_MSC_VER)
        struct tm ptm;
        localtime_s(&ptm, &dVal);
        ptm2 = &ptm;
    #else
        ptm2 = localtime(&dVal);
    #endif

	char buffer[32];

    switch(nType)
            {
            case GlobalType::DateTime:
                strftime (buffer, 32, "%Y-%m-%d %H:%M:%S", ptm2);
                break;
            case GlobalType::DateTimeNoSec:
                strftime (buffer, 32, "%Y-%m-%d %H:%M", ptm2);
                break;
            case GlobalType::Date:
                strftime (buffer, 32, "%Y-%m-%d", ptm2);
                break;
            case GlobalType::Time:
                strftime (buffer, 32, "%H:%M:%S", ptm2);
                break;
            default:
                info._tag_value = "";
            }

        std::string s(buffer);
        info._tag_value.assign(s.begin(), s.end());

        info._tag_type = nType;

        db_xml_info_arr_.push_back(info);
	}


///////////////////////////////////////////////////////////////
//
//
//
string XmlDataGenerator::GetValue(const char* tgName)
{
    DataInfo info;
    for(std::size_t i = 0; i < db_xml_info_arr_.size(); i++)
        {
            info = db_xml_info_arr_.at(i);
            if(info._tag_name == tgName)
                return info._tag_value;
        }
    return "";
}

#ifndef TIDA_OS_ANDROID
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
        doc.AddElem("Name", info._tag_name);
        doc.AddElem("Value", info._tag_value);
        doc.AddElem("Type", info._tag_type);
        doc.OutOfElem();
        }

    doc.OutOfElem();

    return doc.GetDoc();
}

#else
string XmlDataGenerator::GenerateXmlData() const
    {
    if(db_xml_info_arr_.size() <=0 )
        return string("");

    QDomDocument doc;
    QDomElement DATA = doc.createElement("DATA");
    doc.appendChild(DATA);

    for(size_t i=0; i<db_xml_info_arr_.size(); i++)
        {
        DataInfo info = db_xml_info_arr_[i];

        QDomElement Param = doc.createElement("Param");
        DATA.appendChild(Param);

        QDomElement Name = doc.createElement("Name");
        Param.appendChild(Name);
        QDomText sName = doc.createTextNode(QString::fromStdString(info._tag_name));
        Name.appendChild(sName);

        QDomElement Value = doc.createElement("Value");
        Param.appendChild(Value);
        QDomText sValue = doc.createTextNode(QString::fromStdString(info._tag_value));
        Value.appendChild(sValue);

        QDomElement Type = doc.createElement("Type");
        Param.appendChild(Type);
        QDomText sType = doc.createTextNode(QString::number(info._tag_type));
        Type.appendChild(sType);

        }


    return doc.toString().toStdString();
    }
#endif

#ifndef TIDA_OS_ANDROID
//=====================
bool XmlDataGenerator::LoadXmlData(const string& xml)
	{
	CMarkup doc;
	if (!doc.SetDoc(xml))
        {
        last_error_msg_ = doc.GetError();
		return false;
        }

	doc.IntoElem(); // into DATA
    while (doc.FindChildElem("Param"))
		{
        doc.IntoElem();
        string paramName, paramValue, paramType;

        if (doc.FindChildElem("Name"))
            paramName = doc.GetChildData();
        if (doc.FindChildElem("Value"))
            paramValue = doc.GetChildData();
        if (doc.FindChildElem("Type"))
            paramType = doc.GetChildData();

        int nParamType = 0;
#if defined(_WIN32) && defined(_MSC_VER)
    nParamType = atoi(paramType.c_str());
#else
    nParamType = strtol(paramType.c_str(), 0, 10);
#endif
        AddDbInfo(paramName.c_str(), paramValue.c_str(), nParamType);

        doc.OutOfElem();
		}

	return true;
	}
#endif
