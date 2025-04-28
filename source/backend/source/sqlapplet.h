#ifndef SQLAPPLET_H
#define SQLAPPLET_H

#include <stdexcept>
#include "include_util.h"
#include "TypeToStringFormatter.h"

namespace {
const char * APPLET_ERR_INIT = "SQLApplet: Applet initilization error!";
const char * APPLET_ERR_WRONG_PATH = "SQLApplet: Wrong applet name!";
const char * APPLET_ERR_NO_DESCRIPTION = "SQLApplet: Description-tag was not found";
const char * APPLET_ERR_PARAM_NO_NAME = "SQLApplet: No 'Name' tag inside the Param";
const char * APPLET_ERR_PARAM_NO_TYPE = "SQLApplet: No 'Type' tag inside the Param";
const char * APPLET_ERR_PARAM_NO_DEFVAL = "SQLApplet: No 'DefVal' tag inside the Param";
const char * APPLET_ERR_PARAM_NO_VALUE = "SQLApplet: m_formattedParamValueList doesn't have param/value pair provided in Applet";
const char * APPLET_ERR_PARAM_NO_CODE = "SQLApplet: SQL-script could not found!";
}

class DataInfo;
class SQLAppletException : public std::runtime_error
{
public:
    explicit SQLAppletException( const char * msg)
        : std::runtime_error( msg )
        , error(msg)
    {
    }
    string GetError() { return error; }

private:
    string error;
};

class SQLApplet
{
public:
    SQLApplet(const char * appletName, map<string, string> formattedParamValueList = {});
    ~SQLApplet(void){}

    void AddDataInfo(const char * paramName, const char * paramValue);
    void AddDataInfo(const char * paramName, int paramValue);
    void AddDataInfo(const char * paramName, double paramValue);
    void AddDataInfo(const char * paramName, bool paramValue);
    void AddDataInfo(const char * paramName, const std::chrono::sys_seconds paramValue, DataInfo::Type nType);
    void AddDataInfo(const char * paramName,  const char * paramValue, DataInfo::Type nType);

public:
    static void InitPathToApplets(const char * appletPath);
    void parse();

    inline string description(){ return description_; }
    inline string sql(){ return m_sqlSource; }

private:
    map<string, string> m_paramValueList;
    TypeToStringFormatter m_formatter;
    string m_appletPath;
    string description_;
    string m_sqlSource;

};

#endif // SQLAPPLET_H
