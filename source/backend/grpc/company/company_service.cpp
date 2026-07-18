#include "company_service.h"

using std::string;
using std::string_view;

// ============================================================================
// Construction
// ============================================================================

CompanyService::CompanyService(string_view appletPath,
                               string_view dbHost, string_view dbUser,
                               string_view dbPass)
    : m_appletPath(appletPath)
    , m_dbHost(dbHost)
    , m_dbUser(dbUser)
    , m_dbPass(dbPass)
    , m_useInternalRepo(true)
{
}

CompanyService::CompanyService(std::unique_ptr<CompanyRepository> repo)
    : m_repo(std::move(repo))
    , m_useInternalRepo(false)
{
}

std::unique_ptr<CompanyRepository> CompanyService::createRepo()
{
    auto conn = std::make_unique<SqlConnection>(
        SA_PostgreSQL_Client, m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
    return std::make_unique<CompanyRepository>(*conn, m_appletPath);
}

// ============================================================================
// CRUD — Add
// ============================================================================

CompanyData CompanyService::addCompany(const CompanyData& data)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        TransactionScope tx(conn);
        CompanyData result = repo.add(data);
        tx.commit();
        return result;
    }
    return m_repo->add(data);
}

// ============================================================================
// CRUD — Update
// ============================================================================

CompanyData CompanyService::editCompany(const CompanyData& data)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        TransactionScope tx(conn);
        CompanyData result = repo.update(data);
        tx.commit();
        return result;
    }
    return m_repo->update(data);
}

// ============================================================================
// CRUD — Delete
// ============================================================================

DeleteResult CompanyService::deleteCompany(string_view uid)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        TransactionScope tx(conn);
        DeleteResult result = repo.remove(uid);
        tx.commit();
        return result;
    }
    return m_repo->remove(uid);
}

// ============================================================================
// Query
// ============================================================================

std::vector<CompanyData> CompanyService::queryCompanies(const CompanyFilter& filter)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        return repo.query(filter);
    }
    return m_repo->query(filter);
}

// ============================================================================
// Find by UID
// ============================================================================

std::optional<CompanyData> CompanyService::getCompanyByUid(string_view uid)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        return repo.findByUid(uid);
    }
    return m_repo->findByUid(uid);
}

// ============================================================================
// Count
// ============================================================================

int64_t CompanyService::countCompanies(const CompanyFilter& filter)
{
    if (m_useInternalRepo) {
        SqlConnection conn(SA_PostgreSQL_Client,
                           m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
        CompanyRepository repo(conn, m_appletPath);
        return repo.count(filter);
    }
    return m_repo->count(filter);
}
