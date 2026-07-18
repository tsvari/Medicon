#include "company_repository.h"

#include "include_backend_util.h"
#include "JsonParameterFormatter.h"

using std::string;
using std::vector;

// ============================================================================
// Construction
// ============================================================================

CompanyRepository::CompanyRepository(SqlConnection& conn, std::string_view appletPath)
    : m_conn(conn)
    , m_appletPath(appletPath)
{
}

string CompanyRepository::sqlPath(const char* name) const
{
    return m_appletPath + name;
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

    data.logo = row.Field("LOGO").asString().GetMultiByteChars();

    return data;
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

    SAString sql(tpl.sql().c_str());
    SACommand cmd(m_conn.connectionSa(), sql);
    m_conn.connect();
    m_conn.setAutoCommit(true);

    // Bind all parameters
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

    SAString sql(tpl.sql().c_str());
    SACommand cmd(m_conn.connectionSa(), sql);
    m_conn.connect();
    m_conn.setAutoCommit(true);

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
    SqlCommand cmd(m_conn, sqlPath("company_delete.sql"));
    m_conn.connect();
    m_conn.setAutoCommit(true);

    cmd.addParameter("UID", uid.data());
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
    if (!filter.field.empty()) params["FILTER_FIELD"] = filter.field;
    if (!filter.value.empty()) params["FILTER_VALUE"] = filter.value;
    params["OFFSET"] = std::to_string(filter.offset);
    params["LIMIT"] = std::to_string(filter.limit);

    SqlQuery cmd(m_conn, sqlPath("company_select.sql"), std::move(params));
    cmd.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);
    m_conn.connect();

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
    SqlQuery cmd(m_conn, sqlPath("company_select_by_uid.sql"));
    m_conn.connect();
    cmd.addParameter("UID", uid.data());

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
    if (!filter.field.empty()) params["FILTER_FIELD"] = filter.field;
    if (!filter.value.empty()) params["FILTER_VALUE"] = filter.value;

    SqlQuery cmd(m_conn, sqlPath("company_count.sql"), std::move(params));
    cmd.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);
    m_conn.connect();

    if (cmd.query()) {
        return cmd.Field("ROW_COUNT").asInt64();
    }
    return 0;
}
