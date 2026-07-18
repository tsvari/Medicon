#include "company_service.h"

using std::string;
using std::string_view;

// ============================================================================
// Construction
// ============================================================================

CompanyService::CompanyService(string_view appletPath,
                               string_view dbHost, string_view dbUser,
                               string_view dbPass,
                               bool logSql)
    : m_appletPath(appletPath)
    , m_dbHost(dbHost)
    , m_dbUser(dbUser)
    , m_dbPass(dbPass)
    , m_logSql(logSql)
    , m_useInternalRepo(true)
{
}

CompanyService::CompanyService(std::unique_ptr<CompanyRepository> repo)
    : m_repo(std::move(repo))
    , m_useInternalRepo(false)
{
}

// ============================================================================
// Connection management — lazy init + auto-reconnect
// ============================================================================

void CompanyService::ensureConnected()
{
    if (!m_useInternalRepo) {
        return;  // Testing mode — no DB needed
    }

    // Create connection on first access
    if (!m_conn) {
        m_conn = std::make_unique<SqlConnection>(
            SA_PostgreSQL_Client, m_dbHost.c_str(), m_dbUser.c_str(), m_dbPass.c_str());
    }

    // Connect (or reconnect if dropped)
    if (!m_conn->isConnected()) {
        m_conn->connect();
    }
}

// ============================================================================
// CRUD — Add (with transaction)
// ============================================================================

CompanyData CompanyService::addCompany(const CompanyData& data)
{
    if (!m_useInternalRepo) {
        return m_repo->add(data);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    TransactionScope tx(*m_conn);
    CompanyData result = repo.add(data);
    tx.commit();
    return result;
}

// ============================================================================
// CRUD — Update (with transaction)
// ============================================================================

CompanyData CompanyService::editCompany(const CompanyData& data)
{
    if (!m_useInternalRepo) {
        return m_repo->update(data);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    TransactionScope tx(*m_conn);
    CompanyData result = repo.update(data);
    tx.commit();
    return result;
}

// ============================================================================
// CRUD — Delete (with transaction)
// ============================================================================

DeleteResult CompanyService::deleteCompany(string_view uid)
{
    if (!m_useInternalRepo) {
        return m_repo->remove(uid);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    TransactionScope tx(*m_conn);
    DeleteResult result = repo.remove(uid);
    tx.commit();
    return result;
}

// ============================================================================
// Query
// ============================================================================

std::vector<CompanyData> CompanyService::queryCompanies(const CompanyFilter& filter)
{
    if (!m_useInternalRepo) {
        return m_repo->query(filter);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    return repo.query(filter);
}

// ============================================================================
// Find by UID
// ============================================================================

std::optional<CompanyData> CompanyService::getCompanyByUid(string_view uid)
{
    if (!m_useInternalRepo) {
        return m_repo->findByUid(uid);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    return repo.findByUid(uid);
}

// ============================================================================
// Count
// ============================================================================

int64_t CompanyService::countCompanies(const CompanyFilter& filter)
{
    if (!m_useInternalRepo) {
        return m_repo->count(filter);
    }

    ensureConnected();
    CompanyRepository repo(*m_conn, m_appletPath, m_logSql);
    return repo.count(filter);
}
