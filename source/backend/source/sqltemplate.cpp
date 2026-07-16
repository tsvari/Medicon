#include "sqltemplate.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

using std::string;
using std::string_view;
using std::vector;
using std::map;

// ============================================================================
// Construction
// ============================================================================

SqlTemplate::SqlTemplate(string_view filePath)
    : m_filePath(filePath)
{
}

SqlTemplate::SqlTemplate(string_view filePath,
                         map<string, string> formattedParamValueList)
    : m_filePath(filePath)
{
    for (auto& [key, val] : formattedParamValueList) {
        m_formatter.addParameter(key, val);
    }
}

// ============================================================================
// Parameter setters
// ============================================================================

void SqlTemplate::addParameter(string_view name,
                               const std::chrono::milliseconds paramValue,
                               DataInfo::Type nType)
{
    m_formatter.addParameter(name, paramValue, nType);
}

void SqlTemplate::addParameter(string_view name,
                               const char* paramValue,
                               DataInfo::Type nType)
{
    m_formatter.addParameter(name, paramValue, nType);
}

void SqlTemplate::setColumnValidator(string_view paramName,
                                     const ColumnAllowListBase* validator)
{
    m_columnValidators[string(paramName)] = validator;
}

// ============================================================================
// Type name parsing
// ============================================================================

DataInfo::Type SqlTemplate::parseTypeName(string_view typeName)
{
    // Case-insensitive comparison
    string upper;
    upper.reserve(typeName.size());
    for (char c : typeName) {
        upper.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }

    if (upper == "INT" || upper == "NUMERIC")   return DataInfo::Int;
    if (upper == "INT64")                        return DataInfo::Int64;
    if (upper == "STRING")                       return DataInfo::String;
    if (upper == "DOUBLE")                       return DataInfo::Double;
    if (upper == "BOOL" || upper == "BOOLEAN")   return DataInfo::Bool;
    if (upper == "DATETIME")                     return DataInfo::DateTime;
    if (upper == "DATETIMENOSEC")                return DataInfo::DateTimeNoSec;
    if (upper == "DATE")                         return DataInfo::Date;
    if (upper == "TIME")                         return DataInfo::Time;

    throw SqlTemplateException(
        string("Unknown parameter type '") + string(typeName) +
        "'. Valid types: INT, INT64, STRING, DOUBLE, BOOL, DATETIME, "
        "DATETIMENOSEC, DATE, TIME");
}

// ============================================================================
// File loading and header parsing
// ============================================================================

void SqlTemplate::loadAndParseFile()
{
    std::ifstream file(m_filePath);
    if (!file.is_open()) {
        throw SqlTemplateException(
            string("SqlTemplate: Cannot open file: ") + m_filePath);
    }

    string line;
    bool inHeader = true;
    bool foundDescription = false;
    string sqlBody;

    while (std::getline(file, line)) {
        if (inHeader) {
            // Check for -- @param declaration
            if (line.size() > 10 &&
                line[0] == '-' && line[1] == '-' &&
                line[2] == ' ' &&
                line.substr(3, 7) == "@param ")
            {
                // Parse: -- @param NAME TYPE default=VALUE
                string decl = line.substr(10); // after "-- @param "
                
                // Trim trailing whitespace
                auto end = decl.find_last_not_of(" \t\r\n");
                if (end != string::npos) decl.erase(end + 1);
                
                // Split by spaces
                std::istringstream iss(decl);
                string paramName, paramType, defaultToken;
                iss >> paramName >> paramType;
                
                if (paramName.empty()) {
                    throw SqlTemplateException(
                        "SqlTemplate: Parameter name is empty in: " + line);
                }
                if (paramType.empty()) {
                    throw SqlTemplateException(
                        "SqlTemplate: Parameter type missing for '" +
                        paramName + "' in: " + line);
                }

                ParamDecl pd;
                pd.name = paramName;
                pd.type = parseTypeName(paramType);

                // Parse default=VALUE
                string remaining;
                std::getline(iss, remaining);
                auto eqPos = remaining.find('=');
                if (eqPos != string::npos) {
                    // Skip "default" keyword
                    string beforeEq = remaining.substr(0, eqPos);
                    auto trimStart = beforeEq.find_first_not_of(" \t");
                    if (trimStart != string::npos) {
                        beforeEq = beforeEq.substr(trimStart);
                    }
                    // Allow "default=VALUE" or just "=VALUE"
                    if (beforeEq == "default" || beforeEq.empty()) {
                        pd.defaultValue = remaining.substr(eqPos + 1);
                        // Trim quotes from default value
                        if (pd.defaultValue.size() >= 2 &&
                            pd.defaultValue.front() == '\'' &&
                            pd.defaultValue.back() == '\'') {
                            pd.defaultValue = pd.defaultValue.substr(1, pd.defaultValue.size() - 2);
                        }
                        pd.hasDefault = true;
                    }
                }
                
                m_declarations.push_back(std::move(pd));
                continue;
            }
            
            // Check for regular comment (description)
            if (line.size() >= 2 && line[0] == '-' && line[1] == '-') {
                if (!foundDescription) {
                    string desc = line;
                    auto contentStart = desc.find_first_not_of("- \t");
                    if (contentStart != string::npos) {
                        string content = desc.substr(contentStart);
                        // Skip: blank, @param lines, and filename comments (e.g., "-- filename.sql")
                        bool isFilenameComment = (content.find(".sql") != string::npos ||
                                                  content.find(".xml") != string::npos);
                        if (!content.empty() &&
                            content.find("@param") != 0 &&
                            !isFilenameComment) {
                            m_description = content;
                            foundDescription = true;
                        }
                    }
                }
                continue;
            }
            
            // Blank line in header section → still header (skip)
            if (line.find_first_not_of(" \t\r\n") == string::npos) {
                continue;
            }
            
            // First non-comment, non-blank line → end of header, start of SQL
            inHeader = false;
            sqlBody += line;
            sqlBody += '\n';
        } else {
            // SQL body
            sqlBody += line;
            sqlBody += '\n';
        }
    }

    if (sqlBody.empty()) {
        throw SqlTemplateException(
            string("SqlTemplate: No SQL body found in: ") + m_filePath);
    }

    m_rawSql = sqlBody;
    m_fileLoaded = true;
}

// ============================================================================
// Placeholder finding
// ============================================================================

vector<SqlTemplate::ParamPosition>
SqlTemplate::findPlaceholders(string_view sql) const
{
    vector<ParamPosition> positions;
    
    for (size_t i = 0; i < sql.size(); ++i) {
        char c = sql[i];
        Sigil sigil;
        
        if (c == ':' && i + 1 < sql.size()) {
            // Skip PostgreSQL :: cast operator
            if (i + 1 < sql.size() && sql[i + 1] == ':') continue;
            // Skip := (assignment)
            if (i + 1 < sql.size() && sql[i + 1] == '=') continue;
            sigil = Sigil::Bind;
        } else if (c == '$' && i + 1 < sql.size()) {
            // $NAME — identifier placeholder (validated against allow-list)
            sigil = Sigil::Identifier;
        } else {
            continue;
        }
        
        size_t start = i + 1;
        size_t end = start;
        
        // Collect identifier characters
        while (end < sql.size() &&
               (std::isalnum(static_cast<unsigned char>(sql[end])) ||
                sql[end] == '_')) {
            ++end;
        }
        
        if (end > start) {
            ParamPosition pos;
            pos.name = string(sql.substr(start, end - start));
            pos.sigil = sigil;
            pos.offset = i;
            pos.length = end - i;
            positions.push_back(pos);
            i = end - 1;
        }
    }
    
    return positions;
}

// ============================================================================
// Default value formatting
// ============================================================================

string SqlTemplate::formatDefault(const ParamDecl& decl) const
{
    if (!decl.hasDefault) return string();
    
    switch (decl.type) {
        case DataInfo::Int:
        case DataInfo::Int64:
        case DataInfo::Double:
        case DataInfo::Bool:
            return decl.defaultValue;
        case DataInfo::String:
        case DataInfo::DateTime:
        case DataInfo::DateTimeNoSec:
        case DataInfo::Date:
        case DataInfo::Time:
            return "'" + decl.defaultValue + "'";
        default:
            return decl.defaultValue;
    }
}

// ============================================================================
// Main parse() — the core method
// ============================================================================

void SqlTemplate::parse()
{
    // Reset state
    m_sqlSource.clear();
    m_debugSql.clear();
    m_paramBindings.clear();
    m_isParsed = false;

    // Load and parse file if not already loaded
    if (!m_fileLoaded) {
        loadAndParseFile();
    }

    // Get all formatted values from the formatter
    auto formattedValues = m_formatter.toMap();

    string sqlText = m_rawSql;
    string debugText = m_rawSql;

    // Find all :NAME placeholders in SQL
    auto placeholders = findPlaceholders(sqlText);

    string resultSql;
    string resultDebug;
    size_t lastPos = 0;

    for (const auto& ph : placeholders) {
        // Append SQL text before this placeholder
        resultSql.append(sqlText, lastPos, ph.offset - lastPos);
        resultDebug.append(debugText, lastPos, ph.offset - lastPos);

        // Find declaration for this placeholder
        auto declIt = std::find_if(m_declarations.begin(), m_declarations.end(),
            [&](const ParamDecl& d) { return d.name == ph.name; });

        if (declIt == m_declarations.end()) {
            // Unknown placeholder — keep as-is
            char sigil = (ph.sigil == Sigil::Bind) ? ':' : '$';
            resultSql += sigil + ph.name;
            resultDebug += sigil + ph.name;
            lastPos = ph.offset + ph.length;
            continue;
        }

        // Get the formatted value: check addParameter() first, then default
        auto valIt = formattedValues.find(ph.name);
        bool hasValue = (valIt != formattedValues.end());
        string valueStr = hasValue ? valIt->second
                        : (declIt->hasDefault ? formatDefault(*declIt) : string());

        if (ph.sigil == Sigil::Identifier) {
            // $NAME — identifier: validate against allow-list and inline
            if (valueStr.empty() && !hasValue && !declIt->hasDefault) {
                throw SqlTemplateException(
                    "SqlTemplate: Required parameter '" + declIt->name +
                    "' was not provided and has no default value");
            }

            auto valIt2 = m_columnValidators.find(declIt->name);
            if (valIt2 == m_columnValidators.end() || valIt2->second == nullptr) {
                throw SqlTemplateException(
                    "SqlTemplate: No column validator registered for '" +
                    declIt->name + "' in " + m_filePath +
                    ". Use setColumnValidator() before parse().");
            }

            string safeCol = string(valIt2->second->resolve(valueStr));
            resultSql += '"' + safeCol + '"';
            resultDebug += '"' + safeCol + '"';
        } else {
            // :NAME — bind parameter: keep marker for SQLAPI++ binding
            resultSql += ':' + ph.name;

            // For debug SQL, inline the value with proper SQL quoting
            if (!valueStr.empty()) {
                switch (declIt->type) {
                    case DataInfo::String:
                    case DataInfo::DateTime:
                    case DataInfo::DateTimeNoSec:
                    case DataInfo::Date:
                    case DataInfo::Time: {
                        string escaped;
                        escaped.reserve(valueStr.size() + 2);
                        escaped += '\'';
                        for (char c : valueStr) {
                            if (c == '\'') escaped += '\'';
                            escaped += c;
                        }
                        escaped += '\'';
                        resultDebug += escaped;
                        break;
                    }
                    default:
                        resultDebug += valueStr;
                        break;
                }
            } else {
                resultDebug += ':' + ph.name;
            }

            ParamBinding binding;
            binding.name = ':' + ph.name;
            binding.value = valueStr.empty() ? "NULL" : valueStr;
            binding.type = declIt->type;
            m_paramBindings.push_back(std::move(binding));
        }

        lastPos = ph.offset + ph.length;
    }

    // Append remaining SQL text
    resultSql.append(sqlText, lastPos, string::npos);
    resultDebug.append(debugText, lastPos, string::npos);

    m_sqlSource = resultSql;
    m_debugSql = resultDebug;
    m_isParsed = true;
}

// ============================================================================
// Debug SQL
// ============================================================================

string SqlTemplate::getDebugSql() const
{
    if (!m_isParsed) {
        return string();
    }
    return m_debugSql;
}
