#ifndef TYPETOSTRINGFORMATTER_H
#define TYPETOSTRINGFORMATTER_H

#include "include_util.h"
#include <chrono>
#include <variant>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <map>

// Error messages as inline constexpr
inline constexpr const char* ERR_STRING_FORMAT = "Format should be {:%Y-%m-%d %H:%M:%S}!";
inline constexpr const char* ERR_CHRONO_FORMAT = "The provided chronoseconds should be parsed using {:%Y-%m-%d %H:%M:%S}!";
inline constexpr const char* ERR_WRONG_DATE_TIME_TYPE = "The specified argument is not a date or time type!";
inline constexpr const char* ERR_WRONG_KEY_PARAMETER = "Parameter not found: ";
inline constexpr const char* ERR_WRONG_TYPE = "The system does not support this type!";

/**
 * @brief Exception for formatter errors
 */
class FormatterException : public std::runtime_error {
public:
    explicit FormatterException(std::string_view msg)
        : std::runtime_error(std::string(msg)) {}
};

/**
 * @brief Time formatting utilities
 */
namespace timeFormatter {

/**
 * @brief Convert chrono milliseconds to formatted string
 * @throws FormatterException if type is not a date/time type
 */
[[nodiscard]] std::string toString(std::chrono::milliseconds timePoint, DataInfo::Type type);

/**
 * @brief Convert int64_t milliseconds to formatted string
 */
[[nodiscard]] std::string toString(int64_t milliseconds, DataInfo::Type type);

/**
 * @brief Parse formatted string to chrono milliseconds
 * @throws FormatterException if parsing fails
 */
[[nodiscard]] std::chrono::milliseconds fromString(std::string_view formatted, DataInfo::Type type);

/**
 * @brief Get current time as milliseconds
 */
[[nodiscard]] std::chrono::milliseconds now();

/**
 * @brief Generate unique random string (for UUID-like values)
 */
[[nodiscard]] std::string generateUniqueId();

/**
 * @brief Check if type is a date/time type
 */
[[nodiscard]] constexpr bool isDateTimeType(DataInfo::Type type) noexcept {
    return type == DataInfo::DateTime ||
           type == DataInfo::DateTimeNoSec ||
           type == DataInfo::Date ||
           type == DataInfo::Time;
}

} // namespace timeFormatter

/**
 * @brief Type-safe variant for formatter data
 */
using FormatterValue = std::variant<int, int64_t, double, std::string, bool>;

/**
 * @brief Improved TypeToStringFormatter with modern C++ design
 *
 * Key improvements:
 * - Const correctness throughout
 * - string_view for input parameters
 * - Better error messages with context
 * - Optional return types for safe lookups
 * - unordered_map for O(1) lookups by name
 * - Clear naming conventions
 * - Exception safety
 */
class TypeToStringFormatter {
public:
    TypeToStringFormatter() = default;
    ~TypeToStringFormatter() = default;

    // Rule of five
    TypeToStringFormatter(const TypeToStringFormatter&) = default;
    TypeToStringFormatter& operator=(const TypeToStringFormatter&) = default;
    TypeToStringFormatter(TypeToStringFormatter&&) noexcept = default;
    TypeToStringFormatter& operator=(TypeToStringFormatter&&) noexcept = default;

    /**
     * @brief Add parameter with automatic type deduction
     * @throws FormatterException if variant holds unsupported type
     */
    void addParameter(std::string_view name, const FormatterValue& value);

    /**
     * @brief Add date/time parameter from chrono milliseconds
     * @throws FormatterException if type is not a date/time type
     */
    void addParameter(std::string_view name,
                      std::chrono::milliseconds value,
                      DataInfo::Type type);

    /**
     * @brief Add date/time parameter from formatted string
     * @throws FormatterException if parsing fails or type invalid
     */
    void addParameter(std::string_view name,
                      std::string_view value,
                      DataInfo::Type type);

    /**
     * @brief Get formatted parameter value
     * @return Optional containing value if found
     */
    [[nodiscard]] std::optional<std::string_view> getValue(std::string_view name) const;

    /**
     * @brief Get formatted parameter value (throws if not found)
     * @throws FormatterException if parameter not found
     */
    [[nodiscard]] std::string_view getValueOrThrow(std::string_view name) const;

    /**
     * @brief Get parameter info
     * @return Optional containing DataInfo if found
     */
    [[nodiscard]] std::optional<DataInfo> getInfo(std::string_view name) const;

    /**
     * @brief Get parameter info (throws if not found)
     * @throws FormatterException if parameter not found
     */
    [[nodiscard]] const DataInfo& getInfoOrThrow(std::string_view name) const;

    /**
     * @brief Convert parameter to chrono time
     * @throws FormatterException if not found or not a time type
     */
    [[nodiscard]] std::chrono::milliseconds getAsTime(std::string_view name) const;

    /**
     * @brief Get all parameters as name-value map
     */
    [[nodiscard]] std::map<std::string, std::string> toMap() const;

    /**
     * @brief Get all parameter info
     */
    [[nodiscard]] const std::vector<DataInfo>& parameters() const noexcept {
        return m_dataList;
    }

    /**
     * @brief Check if parameter exists
     */
    [[nodiscard]] bool contains(std::string_view name) const;

    /**
     * @brief Get number of parameters
     */
    [[nodiscard]] size_t size() const noexcept {
        return m_dataList.size();
    }

    /**
     * @brief Check if empty
     */
    [[nodiscard]] bool empty() const noexcept {
        return m_dataList.empty();
    }

    /**
     * @brief Clear all parameters
     */
    void clear() noexcept {
        m_dataList.clear();
        m_lookupMap.clear();
    }

private:
    std::vector<DataInfo> m_dataList;
    std::unordered_map<std::string, size_t> m_lookupMap;  // name -> index for O(1) lookup

    void addToList(DataInfo info);
    [[nodiscard]] const DataInfo* findByName(std::string_view name) const;
};

// Backward compatibility aliases
namespace TimeFormatHelper {
using timeFormatter::toString;
using timeFormatter::fromString;
using timeFormatter::now;
using timeFormatter::generateUniqueId;

inline std::string chronoSysSecToString(std::chrono::milliseconds ms, DataInfo::Type type) {
    return toString(ms, type);
}
inline std::string chronoSysSecToString(int64_t ms, DataInfo::Type type) {
    return toString(ms, type);
}
inline std::chrono::milliseconds stringTochronoSysSec(const std::string& str, DataInfo::Type type) {
    return fromString(str, type);
}
inline std::chrono::milliseconds chronoNow() {
    return now();
}
inline std::string generateUniqueString() {
    return generateUniqueId();
}
};

using FormatterDataType = FormatterValue;  // Backward compatibility

#endif // TYPETOSTRINGFORMATTER_H
