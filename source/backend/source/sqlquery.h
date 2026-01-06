#ifndef SQLQUERY_H
#define SQLQUERY_H

#include "sqlcommand.h"

class SqlConnection;

/**
 * @class SqlDirectQuery
 * @brief Direct SQL query execution without applets, with automatic result set fetching
 * 
 * Extends SqlDirectCommand to provide simplified query execution with automatic
 * FetchNext() handling for SELECT queries without requiring XML applets.
 * 
 * Usage:
 * @code
 * SqlConnection conn;
 * SqlDirectQuery query(conn, SAString("SELECT * FROM users WHERE age > 18"));
 * 
 * while(query.query()) {
 *     // Process result set
 *     std::string name = query.Field("name").asString().GetMultiByteChars();
 * }
 * @endcode
 * 
 * For INSERT/UPDATE/DELETE with RETURNING clause (PostgreSQL):
 * @code
 * SqlDirectQuery query(conn, SAString("INSERT INTO users (name) VALUES ('John') RETURNING id"));
 * if(query.query()) {
 *     int64_t id = query.Field("id").asInt64();
 * }
 * @endcode
 */
class SqlDirectQuery : public SqlDirectCommand
{
public:
    /**
     * @brief Construct direct SQL query without applet
     * @param connection Database connection
     * @param sCmd SQL command text
     * @param eCmdType Command type (default SA_CmdUnknown)
     */
    SqlDirectQuery(SqlConnection& connection,
                   const SAString& sCmd,
                   SACommandType_t eCmdType = SA_CmdUnknown);

    // Delete rvalue constructor to prevent dangling references
    SqlDirectQuery(SqlConnection&& connection,
                   const SAString& sCmd,
                   SACommandType_t eCmdType = SA_CmdUnknown) = delete;

    /**
     * @brief Execute query and fetch next row
     * @return true if a row was fetched, false if no more rows
     * @throws SAException if database execution fails
     * 
     * On first call: Executes the query and fetches the first row.
     * On subsequent calls: Fetches the next row from the result set.
     * 
     * Returns true when a row is available for reading.
     * Returns false when no more rows are available or query produced no result set.
     */
    bool query();
};

/**
 * @class SqlQuery
 * @brief SQL query execution with applet support and automatic result set fetching
 * 
 * Extends SqlDirectQuery to add XML applet template support with parameter binding.
 * Inherits the query() method from SqlDirectQuery and adds applet functionality.
 * 
 * Usage:
 * @code
 * SqlConnection conn;
 * SqlQuery query(conn, "select_users.xml");
 * query.addParameter("MinAge", 18);
 * 
 * while(query.query()) {
 *     // Process result set
 *     std::string name = query.Field("name").asString().GetMultiByteChars();
 * }
 * @endcode
 * 
 * For INSERT/UPDATE/DELETE with RETURNING clause (PostgreSQL):
 * @code
 * SqlQuery query(conn, "insert_user.xml");
 * query.addParameter("Name", "John");
 * if(query.query()) {
 *     int64_t id = query.Field("id").asInt64();
 * }
 * @endcode
 */
class SqlQuery : public SqlDirectQuery
{
public:
    /**
     * @brief Construct SQL query from applet with optional parameters
     * @param connection Database connection
     * @param appletName Name of XML applet file (without path)
     * @param formattedParamValueList Pre-formatted parameters (optional)
     * @param sCmd SQL command override (optional)
     * @param eCmdType Command type (default SA_CmdUnknown)
     */
    SqlQuery(SqlConnection & connection,
             const char * appletName,
             std::map<std::string, std::string> formattedParamValueList = {},
             const SAString& sCmd = SAString(),
             SACommandType_t eCmdType = SA_CmdUnknown);

    // Delete rvalue constructor to prevent dangling references
    SqlQuery(SqlConnection&& connection,
             const char * appletName,
             std::map<std::string, std::string> formattedParamValueList = {},
             const SAString& sCmd = SAString(),
             SACommandType_t eCmdType = SA_CmdUnknown) = delete;

    /**
     * @brief Add a parameter with automatic type deduction
     * @tparam T Parameter type (int, int64_t, double, bool, const char*, std::string)
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue Value to substitute
     */
    template<typename T>
    void addParameter(std::string_view name, T paramValue)
    {
        m_applet.addParameter(name, paramValue);
    }

    /**
     * @brief Add a time-based parameter with explicit type formatting
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue Time value in milliseconds since epoch
     * @param nType Format type (DataInfo::Date, DataInfo::Time, DataInfo::DateTime)
     */
    void addParameter(std::string_view name, std::chrono::milliseconds paramValue, DataInfo::Type nType);

    /**
     * @brief Add a string parameter with explicit type formatting
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue String value or time string to parse
     * @param nType Format type (DataInfo::Date, DataInfo::Time, DataInfo::DateTime, etc.)
     */
    void addParameter(std::string_view name, const char* paramValue, DataInfo::Type nType);

    /**
     * @brief Execute the SQL command with substituted parameters
     * @throws SQLAppletException if applet parsing fails
     * @throws SAException if database execution fails
     * 
     * Overrides base class to parse applet template and substitute parameters.
     */
    void execute() override;

    /**
     * @brief Get the final SQL command text with substituted parameters
     * @return SQL command string
     */
    std::string sql() const override;
    
    /**
     * @brief Get SQL with parameters substituted (without executing)
     * @return SQL command string with parameters replaced
     */
    std::string getSqlWithParameters();

private:
    SQLApplet m_applet; ///< Applet managing SQL template and parameters
};

#endif // SQLQUERY_H
