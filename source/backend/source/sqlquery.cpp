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

    // Set the parameterized SQL (with :name markers, not inline values)
    setCommandText(m_applet.sql().c_str());
    
    // Bind all parameters using SQLAPI++ Param() API (SQL injection safe)
    for (const auto& binding : m_applet.paramBindings()) {
        if (binding.value == "NULL") {
            Param(_TSA(binding.name.c_str())).setAsNull();
        } else {
            switch (binding.type) {
                case DataInfo::Int: {
                    // NUMERIC type: bind as long or double depending on format
                    if (binding.value.find('.') != std::string::npos) {
                        Param(_TSA(binding.name.c_str())).setAsDouble() = std::stod(binding.value);
                    } else {
                        Param(_TSA(binding.name.c_str())).setAsLong() = std::stoll(binding.value);
                    }
                    break;
                }
                case DataInfo::Int64: {
                    Param(_TSA(binding.name.c_str())).setAsLong() = std::stoll(binding.value);
                    break;
                }
                case DataInfo::Double: {
                    Param(_TSA(binding.name.c_str())).setAsDouble() = std::stod(binding.value);
                    break;
                }
                case DataInfo::Bool: {
                    bool b = (binding.value == "true" || binding.value == "1");
                    Param(_TSA(binding.name.c_str())).setAsBool() = b;
                    break;
                }
                case DataInfo::String:
                case DataInfo::DateTime:
                case DataInfo::DateTimeNoSec:
                case DataInfo::Date:
                case DataInfo::Time:
                default: {
                    // String and date/time types: bind as string (already formatted)
                    Param(_TSA(binding.name.c_str())).setAsString() = SAString(binding.value.c_str());
                    break;
                }
            }
        }
    }
    
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
    return m_applet.getDebugSql();
}
