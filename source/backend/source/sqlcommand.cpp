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
    
    // TODO: Add easylogging support for SQL execution
    //LOG(INFO) << "Executing SQL: " << m_applet.getDebugSql();
    
    SqlDirectCommand::execute(); // Call base class execute()
}

string SqlCommand::sql() const
{
    return m_applet.sql();
}

string SqlCommand::getSqlWithParameters()
{
    m_applet.parse();
    return m_applet.getDebugSql();
}

