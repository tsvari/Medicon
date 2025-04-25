#ifndef SQLAPPLET_H
#define SQLAPPLET_H

#include <stdexcept>
#include "include_util.h"
#include "TypeToStringFormatter.h"

namespace {
string const APPLET_ERR_WRONG_PATH = "SQLApplet: No path applet name!";
string const APPLET_ERR_NO_DESCRIPTION = "SQLApplet: Description-tag was not found";
string const APPLET_ERR_PARAM_NO_NAME = "SQLApplet: No 'Name' tag inside the Param";
string const APPLET_ERR_PARAM_NO_TYPE = "SQLApplet: No 'Type' tag inside the Param";
string const APPLET_ERR_PARAM_NO_DEFVAL = "SQLApplet: No 'DefVal' tag inside the Param";
string const APPLET_ERR_PARAM_NO_VALUE = "SQLApplet: m_formattedParamValueList doesn't have param/value pair provided in Applet";
string const APPLET_ERR_PARAM_NO_CODE = "SQLApplet: SQL-script could not found!";
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
    SQLApplet(const char * appletName, TypeToStringFormatter const & formatter);
    ~SQLApplet(void){}

public:
    static void InitPathToApplets(const char * appletPath);
    void parse();

    inline string description(){ return description_; }
    inline string sql(){ return m_sqlSource; }

private:
    map<string, string> m_formattedParamValueList;
    string m_appletPath;
    string description_;
    string m_sqlSource;

};

#endif // SQLAPPLET_H
