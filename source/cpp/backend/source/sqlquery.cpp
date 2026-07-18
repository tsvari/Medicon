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

SqlQuery::SqlQuery(SqlConnection& connection,
                   string_view sqlFilePath,
                   map<string, string> formattedParamValueList)
    : SqlDirectQuery(connection, SAString(), SA_CmdUnknown)
    , m_template(sqlFilePath, std::move(formattedParamValueList))
{
}

void SqlQuery::addParameter(string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType)
{
    m_template.addParameter(name, paramValue, nType);
}

void SqlQuery::addParameter(string_view name, const char* paramValue, DataInfo::Type nType)
{
    m_template.addParameter(name, paramValue, nType);
}

void SqlQuery::execute()
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

string SqlQuery::sql() const
{
    return m_template.sql();
}

string SqlQuery::getSqlWithParameters()
{
    m_template.parse();
    return m_template.getDebugSql();
}
