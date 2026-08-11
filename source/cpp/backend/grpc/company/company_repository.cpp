#include "company_repository.h"

#include "include_backend_util.h"
#include "JsonParameterFormatter.h"
#include <easylogging++.h>

using std::string;
using std::vector;

// ============================================================================
// Construction
// ============================================================================

CompanyRepository::CompanyRepository(SqlConnection& conn, std::string_view appletPath,
                                       bool logSql)
    : m_conn(conn)
    , m_appletPath(appletPath)
    , m_logSql(logSql)
{
}

string CompanyRepository::sqlPath(const char* name) const
{
    return m_appletPath + name;
}

void CompanyRepository::ensureConnected()
{
    if (!m_conn.isConnected()) {
        m_conn.connect();
    }
}

// ============================================================================
// Result mapping
// ============================================================================

CompanyData CompanyRepository::rowToCompany(SACommand& row)
{
    CompanyData data;
    data.uid = row.Field("UID").asString().GetMultiByteChars();
    data.server_uid = row.Field("SERVER_UID");
    data.company_type = row.Field("COMPANY_TYPE");

    SAString name = row.Field("NAME").asString();
    name.TrimRight();
    data.name = name.GetMultiByteChars();

    SAString address = row.Field("ADDRESS").asString();
    address.TrimRight();
    data.address = address.GetMultiByteChars();

    string strRegDate = row.Field("REG_DATE").asString().GetMultiByteChars();
    string strJointDate = row.Field("JOINT_DATE").asString().GetMultiByteChars();

    data.reg_date = TimeFormatHelper::stringTochronoSysSec(strRegDate, DataInfo::Date);
    data.joint_date = TimeFormatHelper::stringTochronoSysSec(strJointDate, DataInfo::Date);

    SAString license = row.Field("LICENSE").asString();
    license.TrimRight();
    data.license = license.GetMultiByteChars();

    // LOGO is bytea: asBytes() returns the raw binary (not the hex-encoded
    // text that asString() would). Copy with the explicit byte length —
    // GetMultiByteChars() alone is a C-string and truncates at the first
    // null byte.
    SAString logoBytes = row.Field("LOGO").asBytes();
    data.logo.assign(logoBytes.GetMultiByteChars(),
                     static_cast<size_t>(logoBytes.GetLength()));

    return data;
}

// ============================================================================
// Parameter binding helper
// ============================================================================

void CompanyRepository::bindParams(SACommand& cmd, const SqlTemplate& tpl)
{
    for (const auto& binding : tpl.paramBindings()) {
        if (binding.value == "NULL") {
            cmd.Param(_TSA(binding.name.c_str())).setAsNull();
        } else {
            switch (binding.type) {
                case DataInfo::Int:
                    if (binding.value.find('.') != string::npos)
                        cmd.Param(_TSA(binding.name.c_str())).setAsDouble() = std::stod(binding.value);
                    else
                        cmd.Param(_TSA(binding.name.c_str())).setAsLong() = std::stoll(binding.value);
                    break;
                case DataInfo::Int64:
                    cmd.Param(_TSA(binding.name.c_str())).setAsLong() = std::stoll(binding.value);
                    break;
                case DataInfo::Double:
                    cmd.Param(_TSA(binding.name.c_str())).setAsDouble() = std::stod(binding.value);
                    break;
                case DataInfo::Bool:
                    cmd.Param(_TSA(binding.name.c_str())).setAsBool() = (binding.value == "true" || binding.value == "1");
                    break;
                default:
                    cmd.Param(_TSA(binding.name.c_str())).setAsString() = SAString(binding.value.c_str());
                    break;
            }
        }
    }
}

// ============================================================================
// CRUD — Add
// ============================================================================

CompanyData CompanyRepository::add(const CompanyData& data)
{
    SqlTemplate tpl(sqlPath("company_insert.sql"));
    tpl.addParameter("SERVER_UID", data.server_uid);
    tpl.addParameter("COMPANY_TYPE", data.company_type);
    tpl.addParameter("NAME", data.name.c_str());
    tpl.addParameter("ADDRESS", data.address.c_str());
    tpl.addParameter("REG_DATE", data.reg_date, DataInfo::Date);
    tpl.addParameter("JOINT_DATE", data.joint_date, DataInfo::Date);
    tpl.addParameter("LICENSE", data.license.c_str());
    tpl.parse();

    LOG_IF(m_logSql, INFO) << "[SQL] company_insert: " << tpl.getDebugSql();

    ensureConnected();

    SAString sql(tpl.sql().c_str());
    SACommand cmd(m_conn.connectionSa(), sql);

    bindParams(cmd, tpl);

    // Bind binary logo
    cmd.Param(_TSA("logo")).setAsLongBinary() = SaBinary::toSaString(data.logo);
    cmd.Execute();

    CompanyData result;
    if (cmd.isResultSet() && cmd.FetchNext()) {
        result.uid = cmd.Field("UID").asString().GetMultiByteChars();
    }
    return result;
}

// ============================================================================
// CRUD — Update
// ============================================================================

CompanyData CompanyRepository::update(const CompanyData& data)
{
    SqlTemplate tpl(sqlPath("company_update.sql"));
    tpl.addParameter("UID", data.uid.c_str());
    tpl.addParameter("SERVER_UID", data.server_uid);
    tpl.addParameter("COMPANY_TYPE", data.company_type);
    tpl.addParameter("NAME", data.name.c_str());
    tpl.addParameter("ADDRESS", data.address.c_str());
    tpl.addParameter("REG_DATE", data.reg_date, DataInfo::Date);
    tpl.addParameter("JOINT_DATE", data.joint_date, DataInfo::Date);
    tpl.addParameter("LICENSE", data.license.c_str());
    tpl.parse();

    LOG_IF(m_logSql, INFO) << "[SQL] company_update: " << tpl.getDebugSql();

    ensureConnected();

    SAString sql(tpl.sql().c_str());
    SACommand cmd(m_conn.connectionSa(), sql);

    bindParams(cmd, tpl);

    // Bind binary logo
    cmd.Param(_TSA("logo")).setAsLongBinary() = SaBinary::toSaString(data.logo);
    cmd.Execute();

    CompanyData result;
    if (cmd.isResultSet() && cmd.FetchNext()) {
        result.uid = cmd.Field("UID").asString().GetMultiByteChars();
    }
    return result;
}

// ============================================================================
// CRUD — Delete
// ============================================================================

DeleteResult CompanyRepository::remove(std::string_view uid)
{
    ensureConnected();

    SqlCommand cmd(m_conn, sqlPath("company_delete.sql"));

    cmd.addParameter("UID", uid.data());
    LOG_IF(m_logSql, INFO) << "[SQL] company_delete: " << cmd.getSqlWithParameters();
    cmd.execute();

    DeleteResult result;
    if (cmd.isResultSet() && cmd.FetchNext()) {
        result.uid = cmd.Field("UID").asString().GetMultiByteChars();
        result.success = true;
    } else {
        result.success = false;
        result.error = "No result set";
    }
    return result;
}

// ============================================================================
// Query
// ============================================================================

vector<CompanyData> CompanyRepository::query(const CompanyFilter& filter)
{
    // Build param map from structured filter
    std::map<std::string, std::string> params;
    params["SERVER_UID"] = std::to_string(filter.server_uid);
    if (!filter.field.empty()) params["FILTER_FIELD"] = filter.field;
    if (!filter.value.empty()) params["FILTER_VALUE"] = filter.value;
    params["OFFSET"] = std::to_string(filter.offset);
    params["LIMIT"] = std::to_string(filter.limit);

    ensureConnected();

    SqlQuery cmd(m_conn, sqlPath("company_select.sql"), std::move(params));
    cmd.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);

    LOG_IF(m_logSql, INFO) << "[SQL] company_select: " << cmd.getSqlWithParameters();

    vector<CompanyData> results;
    while (cmd.query()) {
        results.push_back(rowToCompany(cmd));
    }
    return results;
}

// ============================================================================
// Find by UID
// ============================================================================

std::optional<CompanyData> CompanyRepository::findByUid(std::string_view uid)
{
    ensureConnected();

    SqlQuery cmd(m_conn, sqlPath("company_select_by_uid.sql"));
    cmd.addParameter("UID", uid.data());

    LOG_IF(m_logSql, INFO) << "[SQL] company_select_by_uid: " << cmd.getSqlWithParameters();

    if (cmd.query()) {
        return rowToCompany(cmd);
    }
    return std::nullopt;
}

// ============================================================================
// Count
// ============================================================================

int64_t CompanyRepository::count(const CompanyFilter& filter)
{
    std::map<std::string, std::string> params;
    params["SERVER_UID"] = std::to_string(filter.server_uid);
    if (!filter.field.empty()) params["FILTER_FIELD"] = filter.field;
    if (!filter.value.empty()) params["FILTER_VALUE"] = filter.value;

    ensureConnected();

    SqlQuery cmd(m_conn, sqlPath("company_count.sql"), std::move(params));
    cmd.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);

    LOG_IF(m_logSql, INFO) << "[SQL] company_count: " << cmd.getSqlWithParameters();

    if (cmd.query()) {
        return cmd.Field("ROW_COUNT").asInt64();
    }
    return 0;
}
