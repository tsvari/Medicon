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
                       string_view sqlFilePath,
                       map<string, string> formattedParamValueList)
    : SqlDirectCommand(connection, SAString(), SA_CmdUnknown)
    , m_template(sqlFilePath, std::move(formattedParamValueList))
{
}

void SqlCommand::addParameter(string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType)
{
    m_template.addParameter(name, paramValue, nType);
}

void SqlCommand::addParameter(string_view name, const char* paramValue, DataInfo::Type nType)
{
    m_template.addParameter(name, paramValue, nType);
}

void SqlCommand::execute()
{
    try {
        m_template.parse();
    } catch (const SqlTemplateException& e) {
        throw;
    }

    setCommandText(m_template.sql().c_str());

    for (const auto& binding : m_template.paramBindings()) {
        if (binding.value == "NULL") {
            Param(_TSA(binding.name.c_str())).setAsNull();
        } else {
            switch (binding.type) {
                case DataInfo::Int: {
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
                    Param(_TSA(binding.name.c_str())).setAsString() = SAString(binding.value.c_str());
                    break;
                }
            }
        }
    }

    SqlDirectCommand::execute();
}

string SqlCommand::sql() const
{
    return m_template.sql();
}

string SqlCommand::getSqlWithParameters()
{
    m_template.parse();
    return m_template.getDebugSql();
}

