#include "sqlcommand.h"
#include "sqlconnection.h"
// TODO: Add easylogging support
//#include <easylogging++.h>

using std::string;
using std::string_view;
using std::map;

// ============================================================================
// SqlDirectCommand implementation
// ============================================================================

SqlDirectCommand::SqlDirectCommand(SqlConnection& connection,
                                   const SAString& sCmd,
                                   SACommandType_t eCmdType)
    : SACommand(connection.connectionSa(), sCmd, eCmdType)
{
}

void SqlDirectCommand::execute()
{
    Execute(); // Call base class Execute()
    m_executed = true;
}

string SqlDirectCommand::sql() const
{
    return CommandText().GetMultiByteChars();
}

// ============================================================================
// SqlCommand implementation
// ============================================================================

SqlCommand::SqlCommand(SqlConnection& connection,
                       const char* appletName,
                       map<string, string> formattedParamValueList,
                       const SAString& sCmd,
                       SACommandType_t eCmdType)
    : SqlDirectCommand(connection, sCmd, eCmdType)
    , m_applet(appletName, formattedParamValueList)
{
}

void SqlCommand::addParameter(string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType)
{
    m_applet.addParameter(name, paramValue, nType);
}

void SqlCommand::addParameter(string_view name, const char* paramValue, DataInfo::Type nType)
{
    m_applet.addParameter(name, paramValue, nType);
}

void SqlCommand::execute()
{
    try {
        m_applet.parse();
    } catch (const SQLAppletException& e) {
        throw; // Re-throw to preserve exception type
    }

    // TODO: Add easylogging support for SQL execution
    //LOG(INFO) << "Executing SQL: " << m_applet.sql();
    
    setCommandText(m_applet.sql().c_str());
    SqlDirectCommand::execute(); // Call base class execute()
}

string SqlCommand::sql() const
{
    return m_applet.sql();
}

string SqlCommand::getSqlWithParameters()
{
    m_applet.parse();
    return m_applet.sql();
}

