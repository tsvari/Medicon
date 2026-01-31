#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <filesystem>
#include <stdexcept>

/**
 * @brief Data information structure with type metadata
 *
 * Represents a parameter with name, value, and type information.
 * Used for type-safe parameter passing and validation.
 */
struct DataInfo {
    enum Type {
        Zero = 0,
        Int,            // int32_t
        Int64,          // int64_t
        String,         // std::string
        Double,         // double
        Bool,           // bool
        DateTime,       // Full date and time with seconds
        DateTimeNoSec,  // Date and time without seconds
        Date,           // Date only
        Time            // Time only
    };
    std::string param;
    std::string value;
    Type type;
};

// ============================================================================
// String Trimming Utilities
// ============================================================================

namespace Trimmer {
/**
     * @brief Default whitespace characters
     */
inline constexpr const char* ws = " \t\n\r\f\v";

/**
     * @brief Trim whitespace from the end of string (right)
     * @param s String to trim
     * @param t Characters to trim (default: whitespace)
     * @return Trimmed string
     */
[[nodiscard]] std::string rtrim(std::string_view s, const char* t = ws);

/**
     * @brief Trim whitespace from the beginning of string (left)
     * @param s String to trim
     * @param t Characters to trim (default: whitespace)
     * @return Trimmed string
     */
[[nodiscard]] std::string ltrim(std::string_view s, const char* t = ws);

/**
     * @brief Trim whitespace from both ends of string
     * @param s String to trim
     * @param t Characters to trim (default: whitespace)
     * @return Trimmed string
     */
[[nodiscard]] std::string trim(std::string_view s, const char* t = ws);

/**
     * @brief In-place trim from end of string (right)
     * @param s String to modify
     * @param t Characters to trim (default: whitespace)
     * @return Reference to modified string
     */
std::string& rtrimInPlace(std::string& s, const char* t = ws);

/**
     * @brief In-place trim from beginning of string (left)
     * @param s String to modify
     * @param t Characters to trim (default: whitespace)
     * @return Reference to modified string
     */
std::string& ltrimInPlace(std::string& s, const char* t = ws);

/**
     * @brief In-place trim from both ends of string
     * @param s String to modify
     * @param t Characters to trim (default: whitespace)
     * @return Reference to modified string
     */
std::string& trimInPlace(std::string& s, const char* t = ws);
}

// ============================================================================
// Common Utilities
// ============================================================================

namespace CommonUtil {
/**
     * @brief Calculate SQL row offset for pagination
     *
     * Calculates the OFFSET value for SQL LIMIT/OFFSET pagination and
     * provides additional pagination metadata.
     *
     * @param page Current page number (1-based)
     * @param limitCount Number of rows per page
     * @param totalCount Total number of rows in result set
     * @param realCurrentPage [out] Actual current page (may differ if page > pageCount)
     * @param pageCount [out] Total number of pages
     * @return SQL OFFSET value (0-based)
     * @throws std::invalid_argument if limitCount is 0
     *
     * @example
     * uint32_t currentPage, totalPages;
     * auto offset = sqlRowOffset(3, 10, 100, currentPage, totalPages);
     * // offset = 20, currentPage = 3, totalPages = 10
     */
[[nodiscard]] inline std::uint32_t sqlRowOffset(
    std::uint32_t page,
    std::uint32_t limitCount,
    std::uint32_t totalCount,
    std::uint32_t& realCurrentPage,
    std::uint32_t& pageCount
    ) {
    if (limitCount == 0) {
        throw std::invalid_argument("limitCount cannot be zero (division by zero)");
    }

    // If limit is greater than or equal to total, no pagination needed
    if (limitCount >= totalCount) {
        realCurrentPage = 0;
        pageCount = 0;
        return 0;
    }

    // Calculate total number of pages
    pageCount = totalCount / limitCount;
    if (totalCount % limitCount > 0) {
        pageCount++;
    }

    // Validate page number
    if (page == 0 || page > pageCount) {
        realCurrentPage = 0;
        return 0;
    }

    realCurrentPage = page;
    return limitCount * (page - 1);
}

/**
     * @brief Calculate SQL row offset for pagination (simplified)
     *
     * Convenience overload that doesn't return pagination metadata.
     *
     * @param page Current page number (1-based)
     * @param limitCount Number of rows per page
     * @param totalCount Total number of rows in result set
     * @return SQL OFFSET value (0-based)
     * @throws std::invalid_argument if limitCount is 0
     */
[[nodiscard]] inline std::uint32_t sqlRowOffset(
    std::uint32_t page,
    std::uint32_t limitCount,
    std::uint32_t totalCount
    ) {
    std::uint32_t realCurrentPage = 0;
    std::uint32_t pageCount = 0;
    return sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount);
}
}

// ============================================================================
// Binary File Utilities
// ============================================================================

namespace StdBinary {
/**
     * @brief Read entire binary file into string
     * @param path Path to binary file
     * @return File contents as string
     * @throws std::system_error if file cannot be opened
     */
[[nodiscard]] std::string toStdString(const std::filesystem::path& path);

/**
     * @brief Write string data to binary file
     * @param path Path to binary file
     * @param data Data to write
     * @throws std::system_error if file cannot be opened or written
     */
void toBinary(const std::filesystem::path& path, std::string_view data);
}
