#pragma once

#include "company_types.h"
#include "company_column_allowlist.h"
#include "sqlcommand.h"
#include "sqlconnection.h"
#include "sqlquery.h"
#include "sqltemplate.h"

/**
 * @brief Repository layer for company CRUD operations
 *
 * Owns all SQL logic: builds queries, binds parameters,
 * and maps result sets to domain types.
 *
 * Designed to be testable with SQLite in-memory database.
 */
class CompanyRepository {
public:
    CompanyRepository(SqlConnection& conn, std::string_view appletPath);
    virtual ~CompanyRepository() = default;

    // CRUD operations
    virtual CompanyData add(const CompanyData& data);
    virtual CompanyData update(const CompanyData& data);
    virtual DeleteResult remove(std::string_view uid);

    // Query operations
    virtual std::vector<CompanyData> query(const CompanyFilter& filter);
    virtual std::optional<CompanyData> findByUid(std::string_view uid);
    virtual int64_t count(const CompanyFilter& filter);

private:
    [[nodiscard]] std::string sqlPath(const char* name) const;

    // Result mapping helper
    static CompanyData rowToCompany(SACommand& row);

    SqlConnection& m_conn;
    std::string m_appletPath;
};
