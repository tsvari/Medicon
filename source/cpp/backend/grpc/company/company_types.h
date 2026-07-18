#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Domain type for company data (independent of protobuf)
 *
 * Used by CompanyRepository and CompanyService to decouple
 * business logic from gRPC/protobuf types.
 */
struct CompanyData {
    std::string uid;
    int server_uid = 0;
    int company_type = 0;
    std::string name;
    std::string address;
    std::chrono::milliseconds reg_date{0};
    std::chrono::milliseconds joint_date{0};
    std::string license;
    std::string logo;
};

/**
 * @brief Filter parameters for company queries
 */
struct CompanyFilter {
    std::string field;       ///< Column name to filter by (validated via allow-list)
    std::string value;       ///< Search term (LIKE match)
    int offset = 0;
    int limit = 100;
};

/**
 * @brief Result of a delete operation
 */
struct DeleteResult {
    bool success = false;
    std::string uid;         ///< UID of deleted record (if RETURNING)
    std::string error;
};
