#ifndef SQLTEMPLATE_H
#define SQLTEMPLATE_H

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <stdexcept>
#include <vector>

#include "include_util.h"
#include "JsonParameterFormatter.h"
#include "column_allowlist.h"

/**
 * @brief Exception for SqlTemplate errors
 */
class SqlTemplateException : public std::runtime_error
{
public:
    explicit SqlTemplateException(const char* msg)
        : std::runtime_error(msg)
        , error(msg)
    {}
    
    explicit SqlTemplateException(const std::string& msg)
        : std::runtime_error(msg)
        , error(msg)
    {}
    
    [[nodiscard]] std::string_view GetError() const noexcept { return error; }

private:
    std::string error;
};

/**
 * @brief SQL template loaded from a .sql file with parameter declarations
 *
 * SqlTemplate reads SQL files with -- @param header comments that declare
 * parameter names, types, and default values. Supports COLUMN-type parameters
 * validated against explicit allow-lists (replaces the old FIELD type).
 *
 * File format:
 * @code
 * -- company_select.sql
 * -- @param SERVER_UID   NUMERIC  default=0
 * -- @param FILTER_FIELD COLUMN   default=NAME
 * -- @param FILTER_VALUE STRING   default=''
 * --
 * -- Optional description line
 *
 * SELECT * FROM company WHERE "SERVER_UID" = :SERVER_UID
 *   AND :FILTER_FIELD LIKE '%' || :FILTER_VALUE || '%'
 * @endcode
 *
 * Usage:
 * @code
 * SqlTemplate tpl("company_select.sql");
 * tpl.addParameter("SERVER_UID", 123);
 * tpl.addParameter("FILTER_VALUE", "test");
 * tpl.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);
 * tpl.parse();
 *
 * std::string sql = tpl.sql();              // SQL with :NAME markers
 * auto bindings = tpl.paramBindings();      // For SQLAPI++ Param() calls
 * std::string debug = tpl.getDebugSql();    // Human-readable SQL
 * @endcode
 */
class SqlTemplate
{
public:
    /**
     * @brief Parameter binding info (same structure as SQLApplet for compatibility)
     */
    struct ParamBinding {
        std::string name;       ///< Parameter name with colon prefix (e.g., ":SERVER_UID")
        std::string value;      ///< Formatted string value
        DataInfo::Type type;    ///< Declared parameter type
    };

    /**
     * @brief Set global search path for .sql template files
     * @param path Directory path (should end with separator)
     *
     * If set, relative file paths in constructors are resolved against this path.
     * Similar to the old SQLApplet::InitPathToApplets().
     */
    static void setSearchPath(std::string_view path);

    /**
     * @brief Construct SqlTemplate with a .sql file path
     * @param filePath Path to the .sql template file (absolute or relative to search path)
     * @throws SqlTemplateException if file cannot be read
     */
    explicit SqlTemplate(std::string_view filePath);

    /**
     * @brief Construct SqlTemplate with pre-formatted parameters
     * @param filePath Path to the .sql template file
     * @param formattedParamValueList Pre-formatted parameter name→value map
     */
    SqlTemplate(std::string_view filePath,
                std::map<std::string, std::string> formattedParamValueList);

    /**
     * @brief Add a parameter with automatic type deduction
     * @tparam T Parameter type
     * @param name Parameter name (without colon prefix)
     * @param paramValue Value to bind
     */
    template<typename T>
    void addParameter(std::string_view name, T paramValue)
    {
        FormatterDataType data(paramValue);
        m_formatter.addParameter(name, data);
    }
    
    /**
     * @brief Add a time-based parameter with explicit type formatting
     */
    void addParameter(std::string_view name,
                      const std::chrono::milliseconds paramValue,
                      DataInfo::Type nType);
    
    /**
     * @brief Add a string parameter with explicit type formatting
     */
    void addParameter(std::string_view name,
                      const char* paramValue,
                      DataInfo::Type nType);

    /**
     * @brief Register an allow-list validator for a COLUMN-type parameter
     * @param paramName Parameter name (without colon prefix)
     * @param validator Pointer to a ColumnAllowListBase instance
     *
     * Must be called before parse(). The validator must outlive the SqlTemplate.
     */
    void setColumnValidator(std::string_view paramName,
                            const ColumnAllowListBase* validator);

    /**
     * @brief Parse the .sql file, validate parameters, generate SQL with markers
     * @throws SqlTemplateException on file error, missing param, or COLUMN validation failure
     *
     * Reads the file (if not already cached), processes header declarations,
     * substitutes COLUMN-type params inline, and generates:
     * - sql(): SQL with :NAME markers for SQLAPI++ binding
     * - paramBindings(): parameter info for the binding loop
     * - getDebugSql(): human-readable SQL with all values inlined
     */
    void parse();

    /**
     * @brief Get generated SQL with :NAME markers for SQLAPI++ Param() binding
     */
    [[nodiscard]] const std::string& sql() const noexcept { return m_sqlSource; }
    
    /**
     * @brief Get parameter bindings for SQLAPI++ Param() calls
     */
    [[nodiscard]] const std::vector<ParamBinding>& paramBindings() const noexcept
    {
        return m_paramBindings;
    }
    
    /**
     * @brief Get human-readable SQL with all values inlined (for debugging/logging)
     */
    [[nodiscard]] std::string getDebugSql() const;
    
    /**
     * @brief Get applet description (first non-@param comment line)
     */
    [[nodiscard]] const std::string& description() const noexcept { return m_description; }
    
    /**
     * @brief Check if the template has been parsed
     */
    [[nodiscard]] bool isParsed() const noexcept { return m_isParsed; }
    
    /**
     * @brief Get the template file path
     */
    [[nodiscard]] const std::string& filePath() const noexcept { return m_filePath; }

private:
    /**
     * @brief Parsed parameter declaration from -- @param header lines
     */
    struct ParamDecl {
        std::string name;           ///< e.g., "SERVER_UID"
        DataInfo::Type type;        ///< Mapped from type string in header
        std::string defaultValue;   ///< Raw default value string
        bool hasDefault = false;
    };

    /**
     * @brief Sigil type for SQL placeholders
     */
    enum class Sigil { Bind, Identifier };

    /**
     * @brief Position of a placeholder in SQL text
     */
    struct ParamPosition {
        std::string name;       ///< e.g., "SERVER_UID" (without sigil)
        Sigil sigil;            ///< :NAME (bind) or $NAME (identifier)
        size_t offset;           ///< Byte offset in SQL text (including sigil)
        size_t length;           ///< Length including the sigil
    };

    /**
     * @brief Map a type string from the header to DataInfo::Type
     */
    [[nodiscard]] static DataInfo::Type parseTypeName(std::string_view typeName);

    /**
     * @brief Read the .sql file and parse header + SQL body
     */
    void loadAndParseFile();

    /**
     * @brief Find all :NAME placeholders in SQL text
     */
    [[nodiscard]] std::vector<ParamPosition> findPlaceholders(std::string_view sql) const;

    /**
     * @brief Format a default value according to its type
     */
    [[nodiscard]] std::string formatDefault(const ParamDecl& decl) const;

    static std::string s_searchPath;

    std::string m_filePath;
    std::string m_description;
    std::string m_rawSql;           // SQL body from file (with :NAME placeholders)
    std::string m_sqlSource;        // Output SQL with markers (or inlined COLUMNs)
    std::string m_debugSql;         // For logging
    
    bool m_isParsed = false;
    bool m_fileLoaded = false;
    
    // Parameters added by caller
    JsonParameterFormatter m_formatter;
    
    // Parsed declarations from -- @param lines
    std::vector<ParamDecl> m_declarations;
    
    // Output bindings for SQLAPI++ Param()
    std::vector<ParamBinding> m_paramBindings;
    
    // Allow-lists for COLUMN-type params
    std::map<std::string, const ColumnAllowListBase*, std::less<>> m_columnValidators;
};

#endif // SQLTEMPLATE_H
