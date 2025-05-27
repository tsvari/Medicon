#include "sqlcommand.h"
#include "sqlconnection.h"
#include <easylogging++.h>

SqlCommand::SqlCommand(SqlConnection & connection,
                       const char * appletName,
                       map<string, string> formattedParamValueList,
                       const SAString & sCmd,
                       SACommandType_t eCmdType)
    : SACommand(connection.connectionSa(), sCmd, eCmdType)
    , m_applet(appletName, formattedParamValueList)
{

}

void SqlCommand::addDataInfo(const char * paramName, const char * paramValue)
{
    m_applet.AddDataInfo(paramName, paramValue);
}

void SqlCommand::addDataInfo(const char * paramName, int paramValue)
{
    m_applet.AddDataInfo(paramName, paramValue);
}

void SqlCommand::addDataInfo(const char * paramName, double paramValue)
{
    m_applet.AddDataInfo(paramName, paramValue);
}

void SqlCommand::addDataInfo(const char * paramName, bool paramValue)
{
    m_applet.AddDataInfo(paramName, paramValue);
}

void SqlCommand::addDataInfo(const char * paramName, const std::chrono::sys_seconds paramValue, DataInfo::Type nType)
{
    m_applet.AddDataInfo(paramName, paramValue, nType);
}

void SqlCommand::addDataInfo(const char *paramName, const char *paramValue, DataInfo::Type nType)
{
    m_applet.AddDataInfo(paramName, paramValue, nType);
}

void SqlCommand::execute()
{
    m_applet.parse();
    LOG(INFO) << m_applet.sql();
    setCommandText(m_applet.sql().c_str());
    Execute();
}

string SqlCommand::sql()
{
    return m_applet.sql();
}

