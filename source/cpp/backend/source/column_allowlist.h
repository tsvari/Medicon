#ifndef COLUMN_ALLOWLIST_H
#define COLUMN_ALLOWLIST_H

#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <string_view>

/**
 * @brief Base class for column allow-list validators (type erasure)
 */
class ColumnAllowListBase {
public:
    virtual ~ColumnAllowListBase() = default;

    /**
     * @brief Validate and resolve a column name against the allow-list
     * @param name The column name to validate
     * @return The validated column name if found
     * @throws std::invalid_argument if the column name is not in the allow-list
     */
    [[nodiscard]] virtual std::string_view resolve(std::string_view name) const = 0;

    /**
     * @brief Get all valid column names as a comma-separated string (for error messages)
     */
    [[nodiscard]] virtual std::string validNames() const = 0;
};

/**
 * @brief Compile-time column allow-list validator
 *
 * Usage:
 * @code
 * static constexpr ColumnAllowList COMPANY_COLUMNS({
 *     "UID", "SERVER_UID", "COMPANY_TYPE", "NAME",
 *     "ADDRESS", "REG_DATE", "JOINT_DATE", "LICENSE", "LOGO"
 * });
 *
 * std::string_view safe = COMPANY_COLUMNS.resolve(request.column());
 * @endcode
 */
template <std::size_t N>
class ColumnAllowList : public ColumnAllowListBase {
public:
    /**
     * @brief Construct from an initializer list of valid column names
     */
    constexpr ColumnAllowList(std::array<std::string_view, N> valid)
        : m_valid(valid)
    {
        // Sort for binary search (constexpr in C++20)
        std::sort(m_valid.begin(), m_valid.end());
    }

    /**
     * @brief Validate a column name against the allow-list
     * @param name The column name to validate
     * @return The validated column name
     * @throws std::invalid_argument with descriptive error on mismatch
     */
    [[nodiscard]] std::string_view resolve(std::string_view name) const override
    {
        auto it = std::lower_bound(m_valid.begin(), m_valid.end(), name);
        if (it != m_valid.end() && *it == name) {
            return *it;
        }
        throw std::invalid_argument(
            std::string("Unknown column '") + std::string(name) +
            "'. Valid columns: " + validNames());
    }

    /**
     * @brief Get comma-separated list of valid column names
     */
    [[nodiscard]] std::string validNames() const override
    {
        std::string result;
        for (size_t i = 0; i < N; ++i) {
            if (i > 0) result += ", ";
            result += m_valid[i];
        }
        return result;
    }

    /**
     * @brief Check if a name is in the allow-list (without throwing)
     */
    [[nodiscard]] bool contains(std::string_view name) const noexcept
    {
        auto it = std::lower_bound(m_valid.begin(), m_valid.end(), name);
        return it != m_valid.end() && *it == name;
    }

    /**
     * @brief Get the number of valid columns
     */
    [[nodiscard]] constexpr size_t size() const noexcept { return N; }

    /**
     * @brief Iterator access for range-based for
     */
    [[nodiscard]] constexpr auto begin() const noexcept { return m_valid.begin(); }
    [[nodiscard]] constexpr auto end() const noexcept   { return m_valid.end(); }

private:
    std::array<std::string_view, N> m_valid;
};

#endif // COLUMN_ALLOWLIST_H
