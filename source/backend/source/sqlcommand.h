#ifndef SQLCOMMAND_H
#define SQLCOMMAND_H

#include <SQLAPI.h>
#include <string>
#include <string_view>
#include <map>
#include <chrono>

#include "sqlapplet.h"
#include "include_util.h"

class SqlConnection;

/**
 * @class SqlDirectCommand
 * @brief Base class for direct SQL execution without applets
 * 
 * Provides basic SQL execution wrapper. Use this when you don't need
 * parameter substitution or applet templates.
 */
class SqlDirectCommand : public SACommand
{
public:
    SqlDirectCommand(SqlConnection& connection,
                     const SAString& sCmd,
                     SACommandType_t eCmdType = SA_CmdUnknown);

    // Delete rvalue constructor
    SqlDirectCommand(SqlConnection&& connection, 
                     const SAString& sCmd,
                     SACommandType_t eCmdType = SA_CmdUnknown) = delete;

    /**
     * @brief Execute the SQL command (lowercase wrapper)
     * @throws SAException if database execution fails
     * 
     * Wrapper for SACommand::Execute() providing consistent lowercase API.
     */
    virtual void execute();

    virtual std::string sql() const;

protected:
    bool m_executed = false; ///< TODO: Add re-execution prevention
};

/**
 * @class SqlCommand
 * @brief SQL command execution wrapper with parameter binding via applets
 * 
 * Inherits from SqlDirectCommand and adds type-safe parameter binding
 * through SQLApplet integration. Supports automatic type deduction
 * and explicit date/time formatting.
 * 
 * @example
 * SqlConnection conn;
 * SqlCommand cmd(conn, "insert_user.xml");
 * cmd.addParameter("Name", "John");
 * cmd.addParameter("Age", 30);
 * cmd.addParameter("BirthDate", dateMillis, DataInfo::Date);
 * cmd.execute();
 */
class SqlCommand : public SqlDirectCommand
{
public:
    /**
     * @brief Construct SQL command from applet with optional parameters
     * @param connection Database connection
     * @param appletName Name of XML applet file (without path)
     * @param formattedParamValueList Pre-formatted parameters (optional)
     * @param sCmd SQL command override (optional)
     * @param eCmdType Command type (default SA_CmdUnknown)
     */
    SqlCommand(SqlConnection& connection,
               const char* appletName,
               std::map<std::string, std::string> formattedParamValueList = {},
               const SAString& sCmd = SAString(),
               SACommandType_t eCmdType = SA_CmdUnknown);

    // Delete rvalue constructor to prevent dangling references
    SqlCommand(SqlConnection&& connection, 
               const char* appletName, 
               std::map<std::string, std::string> formattedParamValueList = {},
               const SAString& sCmd = SAString(),
               SACommandType_t eCmdType = SA_CmdUnknown) = delete;

    /**
     * @brief Add a parameter with automatic type deduction
     * @tparam T Parameter type (int, int64_t, double, bool, const char*, std::string)
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue Value to substitute
     *
     * Supports: int, int64_t, double, bool, const char*, std::string
     * For date/time types, use the overload with DataInfo::Type
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
     *
     * Formats the time value according to nType before substitution
     */
    void addParameter(std::string_view name, std::chrono::milliseconds paramValue, DataInfo::Type nType);

    /**
     * @brief Add a string parameter with explicit type formatting
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue String value or time string to parse
     * @param nType Format type (DataInfo::Date, DataInfo::Time, DataInfo::DateTime, etc.)
     *
     * For date/time types, parses paramValue as time string and formats accordingly
     */
    void addParameter(std::string_view name, const char* paramValue, DataInfo::Type nType);

    /**
     * @brief Execute the SQL command with substituted parameters
     * @throws SQLAppletException if applet parsing fails
     * @throws SAException if database execution fails
     * 
     * Parses the applet template, substitutes parameters, and executes the command.
     */
    void execute() override;

    /**
     * @brief Get the final SQL command text with substituted parameters
     * @return SQL command string
     * 
     * Overrides base class to return applet-processed SQL.
     */
    std::string sql() const override;
    
    /**
     * @brief Get SQL with parameters substituted (without executing)
     * @return SQL command string with parameters replaced
     * 
     * Useful for testing and debugging. Parses applet if not already parsed.
     */
    std::string getSqlWithParameters();

private:
    SQLApplet m_applet; ///< Applet managing SQL template and parameters
};

#endif // SQLCOMMAND_H
