/**
 * @file CompanyRepositoryPostgresTests.cpp
 * @brief Real PostgreSQL integration tests for CompanyRepository + CompanyService
 *
 * Exercises the actual data layer against a live PostgreSQL instance using the
 * same provider.json credentials the provider uses. Covers the regressions that
 * SQLite/template-only tests cannot: named-parameter binding, empty-string
 * filters, SERVER_UID propagation, bytea round-trips, and transactions.
 *
 * Skipped when:
 *   - MEDICON_SKIP_DB_TESTS is set, or
 *   - the provider config is missing, or
 *   - PostgreSQL is unreachable
 *
 * All tests use a dedicated SERVER_UID (424242) and delete every row they
 * create in TearDown, so the database is left clean.
 */
#include "company/company_repository.h"
#include "company/company_service.h"
#include "configfile.h"
#include "gtest/gtest.h"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr int TEST_SERVER_UID = 30001;
constexpr const char* TEST_LICENSE = "LIC-TEST";

CompanyData makeCompany(const std::string& name, const std::string& logo = "")
{
    CompanyData d;
    d.server_uid = TEST_SERVER_UID;
    d.company_type = 0;
    d.name = name;
    d.address = "123 Test St";
    d.reg_date = std::chrono::milliseconds(1577836800000LL);   // 2020-01-01
    d.joint_date = std::chrono::milliseconds(1593561600000LL); // 2020-07-01
    d.license = TEST_LICENSE;
    d.logo = logo;
    return d;
}

} // namespace

class CompanyRepositoryPostgresTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const char* skip = std::getenv("MEDICON_SKIP_DB_TESTS");
        if (skip && skip[0] != '\0') {
            GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
        }

        // Credentials come from the same provider.json the provider uses
        ConfigFile cfg(ALL_PROJECT_APPDATA_PATH, "provider");
        cfg.load();
        m_host = cfg.value("host");
        m_user = cfg.value("user");
        m_pass = cfg.value("pass");
        m_appletPath = cfg.appletPath();

        if (m_host.empty() || m_user.empty() || m_appletPath.empty()) {
            GTEST_SKIP() << "provider DB config incomplete (host/user/appletPath)";
        }

        // Connect eagerly — if PostgreSQL is down, skip rather than fail
        m_conn = std::make_unique<SqlConnection>(
            SA_PostgreSQL_Client, m_host.c_str(), m_user.c_str(), m_pass.c_str());
        try {
            m_conn->connect();
        } catch (...) {
            GTEST_SKIP() << "PostgreSQL unreachable (host=" << m_host << ")";
        }
    }

    void TearDown() override
    {
        if (m_conn && m_conn->isConnected()) {
            try {
                std::string sql = "DELETE FROM company WHERE \"SERVER_UID\" = " +
                                  std::to_string(TEST_SERVER_UID);
                SqlDirectCommand cleanup(*m_conn, SAString(sql.c_str()));
                cleanup.execute();
            } catch (...) {
                // Best-effort cleanup — never fail the test here
            }
        }
    }

    SqlConnection& conn() { return *m_conn; }

    std::unique_ptr<SqlConnection> m_conn;
    std::string m_host, m_user, m_pass, m_appletPath;
};

// ============================================================================
// CRUD round-trips
// ============================================================================

TEST_F(CompanyRepositoryPostgresTest, Add_ReturnsUid_AndFindByUid_RoundTrips)
{
    CompanyRepository repo(conn(), m_appletPath, /*logSql=*/false);
    CompanyData added = repo.add(makeCompany("PostgresTest Corp"));
    ASSERT_FALSE(added.uid.empty()) << "INSERT ... RETURNING UID must yield a uid";

    auto found = repo.findByUid(added.uid);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->server_uid, TEST_SERVER_UID);
    EXPECT_EQ(found->name, "PostgresTest Corp");
    EXPECT_EQ(found->address, "123 Test St");
    EXPECT_EQ(found->license, TEST_LICENSE);
    EXPECT_EQ(found->company_type, 0);
}

TEST_F(CompanyRepositoryPostgresTest, Update_ModifiesRow)
{
    CompanyRepository repo(conn(), m_appletPath, false);
    CompanyData added = repo.add(makeCompany("Before Name"));
    ASSERT_FALSE(added.uid.empty());

    added.name = "After Name";
    added.address = "456 New Ave";
    CompanyData updated = repo.update(added);
    ASSERT_FALSE(updated.uid.empty());

    auto found = repo.findByUid(added.uid);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "After Name");
    EXPECT_EQ(found->address, "456 New Ave");
}

TEST_F(CompanyRepositoryPostgresTest, Delete_RemovesRow)
{
    CompanyRepository repo(conn(), m_appletPath, false);
    CompanyData added = repo.add(makeCompany("ToDelete"));
    ASSERT_FALSE(added.uid.empty());

    DeleteResult del = repo.remove(added.uid);
    EXPECT_TRUE(del.success);

    EXPECT_FALSE(repo.findByUid(added.uid).has_value())
        << "row must be gone after delete";
}

// ============================================================================
// Filter regression: SERVER_UID propagation + empty FILTER_VALUE
// ============================================================================

TEST_F(CompanyRepositoryPostgresTest, Count_WithServerUidAndEmptyFilter_ReturnsInsertedCount)
{
    CompanyRepository repo(conn(), m_appletPath, false);
    const int rows = 5;
    for (int i = 0; i < rows; ++i) {
        CompanyData added = repo.add(makeCompany("Count " + std::to_string(i)));
        ASSERT_FALSE(added.uid.empty());
    }

    CompanyFilter filter;
    filter.server_uid = TEST_SERVER_UID;  // regression: was dropped -> SERVER_UID=0
    filter.field = "NAME";
    filter.value = "";                    // regression: empty -> NULL -> 0 rows

    EXPECT_EQ(repo.count(filter), rows);
}

TEST_F(CompanyRepositoryPostgresTest, Query_WithServerUidAndEmptyFilter_ReturnsAll)
{
    CompanyRepository repo(conn(), m_appletPath, false);
    const int rows = 3;
    for (int i = 0; i < rows; ++i) {
        repo.add(makeCompany("Query " + std::to_string(i)));
    }

    CompanyFilter filter;
    filter.server_uid = TEST_SERVER_UID;
    filter.field = "NAME";
    filter.value = "";
    filter.offset = 0;
    filter.limit = 100;

    auto results = repo.query(filter);
    EXPECT_EQ(results.size(), rows);
}

TEST_F(CompanyRepositoryPostgresTest, Query_WithFilterValue_FiltersRows)
{
    CompanyRepository repo(conn(), m_appletPath, false);
    repo.add(makeCompany("Alpha"));
    repo.add(makeCompany("Beta"));

    CompanyFilter filter;
    filter.server_uid = TEST_SERVER_UID;
    filter.field = "NAME";
    filter.value = "Alp";

    auto results = repo.query(filter);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].name, "Alpha");
}

// ============================================================================
// bytea LOGO regression
// ============================================================================

TEST_F(CompanyRepositoryPostgresTest, Logo_BinaryRoundTrip_PreservesBytes)
{
    // PNG-style header + embedded null bytes + high bytes — must round-trip exactly
    std::string logo;
    logo += "\x89PNG\r\n\x1A\n";
    logo += std::string("\x00\x00\x01", 3);
    logo += "binary\xFF\xFE\x00payload";

    CompanyRepository repo(conn(), m_appletPath, false);
    CompanyData added = repo.add(makeCompany("LogoTest", logo));
    ASSERT_FALSE(added.uid.empty());

    auto found = repo.findByUid(added.uid);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->logo, logo)
        << "bytea logo must round-trip byte-for-byte (was hex-encoded/truncated)";
}

// ============================================================================
// Service layer: TransactionScope + ensureConnected against real PostgreSQL
// ============================================================================

TEST_F(CompanyRepositoryPostgresTest, Service_AddQueryDelete_UsesTransaction)
{
    CompanyService service(m_appletPath, m_host, m_user, m_pass, /*logSql=*/false);

    CompanyData added = service.addCompany(makeCompany("ServiceTx"));
    ASSERT_FALSE(added.uid.empty());

    auto found = service.getCompanyByUid(added.uid);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "ServiceTx");

    DeleteResult del = service.deleteCompany(added.uid);
    EXPECT_TRUE(del.success);
    EXPECT_FALSE(service.getCompanyByUid(added.uid).has_value());
}

TEST_F(CompanyRepositoryPostgresTest, Service_QueryCompanies_WithServerUidAndEmptyFilter)
{
    CompanyService service(m_appletPath, m_host, m_user, m_pass, false);
    service.addCompany(makeCompany("Srv A"));
    service.addCompany(makeCompany("Srv B"));

    CompanyFilter filter;
    filter.server_uid = TEST_SERVER_UID;
    filter.field = "NAME";
    filter.value = "";

    auto list = service.queryCompanies(filter);
    EXPECT_EQ(list.size(), 2u);
}
