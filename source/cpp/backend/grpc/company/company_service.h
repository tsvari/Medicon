#pragma once

#include "company_repository.h"
#include "company_types.h"
#include "sqlconnection.h"
#include "transactionscope.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Business logic layer for company operations
 *
 * Manages SqlConnection lifecycles, coordinates transactions via TransactionScope,
 * and translates repository-level errors to domain results.
 *
 * Designed for testability: accepts an optional pre-built repository
 * for unit testing with mock data.
 */
class CompanyService {
public:
    /**
     * @brief Construct with connection parameters (normal mode)
     */
    CompanyService(std::string_view appletPath,
                   std::string_view dbHost,
                   std::string_view dbUser,
                   std::string_view dbPass);

    /**
     * @brief Construct with pre-built repository (testing mode)
     */
    explicit CompanyService(std::unique_ptr<CompanyRepository> repo);

    // CRUD
    CompanyData addCompany(const CompanyData& data);
    CompanyData editCompany(const CompanyData& data);
    DeleteResult deleteCompany(std::string_view uid);

    // Queries
    std::vector<CompanyData> queryCompanies(const CompanyFilter& filter);
    std::optional<CompanyData> getCompanyByUid(std::string_view uid);
    int64_t countCompanies(const CompanyFilter& filter);

private:
    /**
     * @brief Ensure a shared database connection is established (lazy + reconnect)
     *
     * Creates the connection on first call and reconnects if the connection
     * was dropped. No-op in testing mode (injected repository).
     * @throws SAException if connection fails
     */
    void ensureConnected();

    std::unique_ptr<SqlConnection> m_conn;  ///< Shared connection (internal mode)
    std::unique_ptr<CompanyRepository> m_repo;  ///< Injected repo (for testing)
    std::string m_appletPath;
    std::string m_dbHost;
    std::string m_dbUser;
    std::string m_dbPass;
    bool m_useInternalRepo = true;  ///< false when repo is injected
};
