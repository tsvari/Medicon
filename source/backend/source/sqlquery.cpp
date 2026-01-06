#include "sqlquery.h"

using std::string;
using std::string_view;
using std::map;

// ============================================================================
// SqlDirectQuery implementation
// ============================================================================

SqlDirectQuery::SqlDirectQuery(SqlConnection& connection,
                               const SAString& sCmd,
                               SACommandType_t eCmdType)
    : SqlDirectCommand(connection, sCmd, eCmdType)
{
}

bool SqlDirectQuery::query()
{
    if(!m_executed) {
        // First call: execute and fetch first row
        execute();
        
        // Check if result set exists before calling FetchNext()
        return isResultSet() && FetchNext();
    } else {
        // Subsequent calls: just fetch next row
        return FetchNext();
    }
}

// ============================================================================
// SqlQuery implementation
// ============================================================================

SqlQuery::SqlQuery(SqlConnection & connection,
                   const char * appletName,
                   map<string, string> formattedParamValueList,
                   const SAString & sCmd,
                   SACommandType_t eCmdType)
    : SqlDirectQuery(connection, sCmd, eCmdType)
    , m_applet(appletName, formattedParamValueList)
{
}

void SqlQuery::addParameter(string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType)
{
    m_applet.addParameter(name, paramValue, nType);
}

void SqlQuery::addParameter(string_view name, const char* paramValue, DataInfo::Type nType)
{
    m_applet.addParameter(name, paramValue, nType);
}

void SqlQuery::execute()
{
    try {
        m_applet.parse();
    } catch (const SQLAppletException& e) {
        throw; // Re-throw to preserve exception type
    }

    // Set the parsed SQL with parameters
    setCommandText(m_applet.sql().c_str());
    
    // Call base class execute (SqlDirectCommand::execute())
    SqlDirectCommand::execute();
}

string SqlQuery::sql() const
{
    return m_applet.sql();
}

string SqlQuery::getSqlWithParameters()
{
    m_applet.parse();
    return m_applet.sql();
}
