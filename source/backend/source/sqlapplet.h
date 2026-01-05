#ifndef SQLAPPLET_H
#define SQLAPPLET_H

#include <stdexcept>
#include "include_util.h"
#include "JsonParameterFormatter.h"

// Error messages
inline constexpr const char* APPLET_ERR_INIT = "SQLApplet: Applet initialization error! Call SQLApplet::InitPathToApplets() first.";
inline constexpr const char* APPLET_ERR_WRONG_PATH = "SQLApplet: Applet file not found at path: ";
inline constexpr const char* APPLET_ERR_NO_DESCRIPTION = "SQLApplet: Description tag was not found in applet XML";
inline constexpr const char* APPLET_ERR_PARAM_NO_NAME = "SQLApplet: Parameter is missing 'Name' tag";
inline constexpr const char* APPLET_ERR_PARAM_NO_TYPE = "SQLApplet: Parameter is missing 'Type' tag";
inline constexpr const char* APPLET_ERR_PARAM_NO_DEFVAL = "SQLApplet: Parameter is missing 'DefVal' tag and no value provided";
inline constexpr const char* APPLET_ERR_PARAM_NO_VALUE = "SQLApplet: Required parameter not provided and default values are disabled";
inline constexpr const char* APPLET_ERR_PARAM_NO_CODE = "SQLApplet: SQL code block not found in applet XML";
inline constexpr const char* APPLET_ERR_PARAM_XML = "SQLApplet: XML parsing error";
inline constexpr const char* APPLET_ERR_WRONG_TYPE_NAME = "SQLApplet: Unknown parameter type. Valid types: FIELD, NUMERIC, STRING, DATETIME, DATE, TIME";

class DataInfo;

/**
 * @brief Exception for SQLApplet errors
 */
class SQLAppletException : public std::runtime_error
{
public:
    explicit SQLAppletException(const char* msg)
        : std::runtime_error(msg)
        , error(msg)
    {}
    
    explicit SQLAppletException(const std::string& msg)
        : std::runtime_error(msg)
        , error(msg)
    {}
    
    std::string_view GetError() const noexcept { return error; }

private:
    std::string error;
};

/**
 * @brief SQL Applet for loading and executing parameterized SQL scripts
 * 
 * SQLApplet loads SQL templates from XML files, substitutes parameters,
 * and generates executable SQL statements. Supports various data types
 * including dates, times, and custom formatting.
 * 
 * Usage:
 * @code
 * SQLApplet::InitPathToApplets("/path/to/applets/");
 * SQLApplet applet("myquery.xml");
 * applet.addParameter("UserId", 123);
 * applet.addParameter("Date", timePoint, DataInfo::Date);
 * applet.parse();
 * std::string sql = applet.sql();
 * @endcode
 */
class SQLApplet
{
public:
    /**
     * @brief Construct SQLApplet with optional pre-formatted parameters
     * @param appletName Name of the XML applet file (relative to InitPathToApplets path)
     * @param formattedParamValueList Optional map of parameter names to string values (already formatted)
     * @throws SQLAppletException if InitPathToApplets() was not called
     * 
     * @note Parameters can be added later using addParameter() before calling parse()
     */
    SQLApplet(std::string_view appletName, std::map<std::string, std::string> formattedParamValueList = {});
    
    /**
     * @brief Construct SQLApplet with JSON-formatted parameters
     * @param appletName Name of the XML applet file (relative to InitPathToApplets path)
     * @param parametersXml JSON string containing parameter name-value pairs
     * @throws SQLAppletException if InitPathToApplets() was not called or JSON is invalid
     * 
     * Example JSON: {"UserId": "123", "Date": "2024-01-01"}
     */
    SQLApplet(std::string_view appletName, const std::string & parametersXml);

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
        FormatterDataType data(paramValue);
        m_formatter.addParameter(name, data);
    }
    
    /**
     * @brief Add a time-based parameter with explicit type formatting
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue Time value in milliseconds since epoch
     * @param nType Format type (DataInfo::Date, DataInfo::Time, DataInfo::DateTime)
     * 
     * Formats the time value according to nType before substitution
     */
    void addParameter(std::string_view name, const std::chrono::milliseconds paramValue, DataInfo::Type nType);
    
    /**
     * @brief Add a string parameter with explicit type formatting
     * @param name Parameter name matching placeholder in XML (:name:)
     * @param paramValue String value or time string to parse
     * @param nType Format type (DataInfo::Date, DataInfo::Time, DataInfo::DateTime, etc.)
     * 
     * For date/time types, parses paramValue as time string and formats accordingly
     */
    void addParameter(std::string_view name, const char * paramValue, DataInfo::Type nType);

public:
    /**
     * @brief Initialize global applet path (must be called before creating any SQLApplet instances)
     * @param appletPath Directory path containing XML applet files (should end with /)
     * @param useDefaultValue If true, use default values from XML when parameters not provided
     * 
     * @note Must be called once at application startup before any SQLApplet usage
     * @note Not thread-safe: ensure all threads complete initialization before creating SQLApplet instances
     */
    static void InitPathToApplets(std::string_view appletPath, bool useDefaultValue = false);

    /**
     * @brief Parse the XML applet and generate SQL with substituted parameters
     * @throws SQLAppletException if file not found, XML malformed, or required parameters missing
     * 
     * Loads XML, validates parameters, substitutes placeholders (:name:) with values,
     * and stores the result accessible via sql()
     * 
     * @note Can be called multiple times, regenerates SQL each time
     */
    void parse();
    
    /**
     * @brief Get applet description
     */
    [[nodiscard]] const std::string& description() const noexcept { return m_description; }
    
    /**
     * @brief Get generated SQL with substituted parameters
     */
    [[nodiscard]] const std::string& sql() const noexcept { return m_sqlSource; }
    
    /**
     * @brief Get applet file path
     */
    [[nodiscard]] const std::string& appletPath() const noexcept { return m_appletPath; }
    
    /**
     * @brief Check if applet has been parsed
     */
    [[nodiscard]] bool isParsed() const noexcept { return m_isParsed; }

private:
    std::map<std::string, std::string> m_paramValueList;
    JsonParameterFormatter m_formatter;
    std::string m_appletPath;
    std::string m_description;
    std::string m_sqlSource;
    bool m_isParsed = false;

};

#endif // SQLAPPLET_H
