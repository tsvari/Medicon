#ifndef JSONPARAMETERFORMATTER_H
#define JSONPARAMETERFORMATTER_H

#include "TypeToStringFormatter.h"
#include <string_view>
#include <map>
#include <optional>

// Error messages
inline constexpr const char* ERR_INVALID_JSON = "Invalid JSON format";
inline constexpr const char* ERR_JSON_NOT_OBJECT = "JSON must be an object (key-value pairs)";

/**
 * @brief Exception for JSON parameter errors
 */
class JsonFormatterException : public FormatterException {
public:
    explicit JsonFormatterException(std::string_view msg)
        : FormatterException(msg) {}
};

// ============================================================================
// JsonParameterFormatter - Main API
// ============================================================================

/**
 * @brief JsonParameterFormatter with modern design and consistent naming
 *
 * Inherits from TypeToStringFormatter to provide all parameter management
 * functionality plus JSON serialization/deserialization.
 *
 * Uses modern C++ naming conventions:
 * - addParameter() for adding parameters (inherited from TypeToStringFormatter)
 * - toJson() for serialization
 * - fromJson() for deserialization
 * - getParameters() for retrieving parameters as map
 *
 * Example usage:
 * @code
 * JsonParameterFormatter formatter;
 * formatter.addParameter("Name", "John");
 * formatter.addParameter("Age", 30);
 *
 * std::string json = formatter.toJson(true);  // Pretty print
 * formatter.fromJson(R"({"City":"New York"})");
 * @endcode
 */
class JsonParameterFormatter : public TypeToStringFormatter {
public:
    JsonParameterFormatter() = default;

    // Inherit all addParameter methods from TypeToStringFormatter
    using TypeToStringFormatter::addParameter;

    // ========================================================================
    // Instance JSON Methods
    // ========================================================================

    /**
     * @brief Serialize this formatter's parameters to JSON
     * @param pretty Pretty-print with indentation (default: false)
     * @return JSON string
     */
    [[nodiscard]] std::string toJson(bool pretty = false) const;

    /**
     * @brief Load parameters from JSON string
     * @param jsonString JSON object string like {"key":"value"}
     * @throws JsonFormatterException if parsing fails
     */
    void fromJson(std::string_view jsonString);

    /**
     * @brief Try to load parameters from JSON string (non-throwing)
     * @param jsonString JSON object string
     * @return true if successful, false otherwise
     */
    [[nodiscard]] bool tryFromJson(std::string_view jsonString) noexcept;

    /**
     * @brief Get parameters as map (convenience method)
     * @return Map of parameter name-value pairs
     */
    [[nodiscard]] std::map<std::string, std::string> getParameters() const {
        return toMap();
    }

    // ========================================================================
    // Static JSON Utilities
    // ========================================================================

    /**
     * @brief Parse JSON string to parameter map
     * @param jsonString JSON object string like {"key":"value"}
     * @return Map of parameter name-value pairs
     * @throws JsonFormatterException if parsing fails or invalid format
     */
    [[nodiscard]] static std::map<std::string, std::string>
    fromJsonString(std::string_view jsonString);

    /**
     * @brief Parse JSON string safely (non-throwing)
     * @param jsonString JSON object string
     * @return Optional containing map if successful, empty otherwise
     */
    [[nodiscard]] static std::optional<std::map<std::string, std::string>>
    tryFromJsonString(std::string_view jsonString) noexcept;

    /**
     * @brief Convert parameter map to JSON string
     * @param parameters Parameter name-value pairs
     * @param pretty Pretty-print with indentation (default: false)
     * @return JSON string
     */
    [[nodiscard]] static std::string
    toJsonString(const std::map<std::string, std::string>& parameters, bool pretty = false);

    /**
     * @brief Convert TypeToStringFormatter to JSON
     * @param formatter Formatter containing parameters
     * @param pretty Pretty-print with indentation
     * @return JSON string
     */
    [[nodiscard]] static std::string
    toJsonString(const TypeToStringFormatter& formatter, bool pretty = false);

    /**
     * @brief Validate JSON string without parsing
     * @param jsonString JSON to validate
     * @return true if valid JSON object with string values
     */
    [[nodiscard]] static bool isValidJson(std::string_view jsonString) noexcept;

    /**
     * @brief Get detailed error message from last parse attempt
     * @param jsonString The JSON string that failed to parse
     * @return Human-readable error with position info
     */
    [[nodiscard]] static std::string getParseError(std::string_view jsonString);
};

#endif // JSONPARAMETERFORMATTER_H
