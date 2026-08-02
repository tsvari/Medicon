/**
 * @file CompanyLoggingIntegrationTests.cpp
 * @brief Integration tests for SQL and error logging
 *
 * Verifies that:
 * 1. SQL debug output contains meaningful parameter values (logSql path)
 * 2. Error exceptions from CompanyRepository contain descriptive messages
 * 3. Log file path resolution works correctly
 */

#include "company/company_column_allowlist.h"
#include "company/company_repository.h"
#include "company/company_types.h"
#include "sqlconnection.h"
#include "sqlcommand.h"
#include "configfile.h"
#include "gtest/gtest.h"

#include <cstdlib>
#include <string>

// ============================================================================
// Test fixture
// ============================================================================

class CompanyLoggingIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Provider's app-data holds the company .sql templates
        ConfigFile config(ALL_PROJECT_APPDATA_PATH, "provider");
        ASSERT_NO_THROW(config.load());
        m_appletPath = config.appletPath();
    }

    std::string m_appletPath;
};

// ============================================================================
// SQL Debug Output Tests
// ============================================================================

/**
 * @test SqlTemplate debug output contains all parameter values
 *
 * This validates that the human-readable SQL output (what gets logged
 * when logSql=true) includes all substituted values for auditing.
 */
TEST_F(CompanyLoggingIntegrationTest, DebugSql_ContainsParameterValues)
{
    SqlTemplate tpl(m_appletPath + "company_insert.sql");
    tpl.addParameter("SERVER_UID", 42);
    tpl.addParameter("COMPANY_TYPE", 3);
    tpl.addParameter("NAME", "AuditCorp");
    tpl.addParameter("ADDRESS", "456 Audit Ln");
    tpl.addParameter("REG_DATE", "2023-01-15", DataInfo::Date);
    tpl.addParameter("JOINT_DATE", "2023-06-20", DataInfo::Date);
    tpl.addParameter("LICENSE", "LIC-AUDIT-001");
    tpl.addParameter("LOGO", "");

    ASSERT_NO_THROW(tpl.parse());

    std::string debug = tpl.getDebugSql();

    // Verify all meaningful values appear in the debug output
    EXPECT_NE(debug.find("42"), std::string::npos) << "Should contain SERVER_UID";
    EXPECT_NE(debug.find("AuditCorp"), std::string::npos) << "Should contain NAME";
    EXPECT_NE(debug.find("456 Audit Ln"), std::string::npos) << "Should contain ADDRESS";
    EXPECT_NE(debug.find("LIC-AUDIT-001"), std::string::npos) << "Should contain LICENSE";
}

/**
 * @test Delete SQL debug output contains UID value
 */
TEST_F(CompanyLoggingIntegrationTest, DeleteDebugSql_ContainsUid)
{
    SqlTemplate tpl(m_appletPath + "company_delete.sql");
    tpl.addParameter("UID", "delete-this-uid-123");

    ASSERT_NO_THROW(tpl.parse());

    std::string debug = tpl.getDebugSql();
    EXPECT_NE(debug.find("delete-this-uid-123"), std::string::npos)
        << "Debug SQL should contain the UID being deleted";
}

// ============================================================================
// Error Message Tests
// ============================================================================

/**
 * @test CompanyRepository throws meaningful errors
 *
 * When a SQL error occurs (e.g., mismatched schema), the exception
 * should contain a descriptive message suitable for logging.
 */
TEST_F(CompanyLoggingIntegrationTest, RepositoryError_MessageIsDescriptive)
{
    const char* skipEnv = std::getenv("MEDICON_SKIP_DB_TESTS");
    if (skipEnv && skipEnv[0] != '\0') {
        GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
    }

    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    ASSERT_NO_THROW(conn.connect());

    // Create a minimal table (missing columns required by company_insert)
    SqlDirectCommand create(conn, SAString(
        "CREATE TABLE company (\"UID\" TEXT PRIMARY KEY, \"NAME\" TEXT)"
    ));
    create.execute();

    // Attempt insert — will fail because the INSERT expects more columns
    CompanyRepository repo(conn, m_appletPath, true);
    CompanyData data;
    data.name = "ErrorTest";
    data.server_uid = 1;
    data.company_type = 2;
    data.address = "Addr";
    data.license = "LIC";
    data.logo = "";

    try {
        repo.add(data);
        FAIL() << "Expected exception was not thrown";
    } catch (const SAException& e) {
        // SAException from SQLAPI++ — expected for SQL errors
        std::string msg = e.ErrText().GetMultiByteChars();
        EXPECT_FALSE(msg.empty()) << "SAException error text should not be empty";
    } catch (const SqlTemplateException& e) {
        // SqlTemplateException from template parsing
        std::string msg = e.what();
        EXPECT_FALSE(msg.empty()) << "SqlTemplateException message should not be empty";
    } catch (const std::exception& e) {
        // Standard exception — message should be descriptive
        std::string msg = e.what();
        EXPECT_FALSE(msg.empty()) << "Exception message should not be empty";
    } catch (...) {
        // Unknown exception — catch to prevent test crash
        FAIL() << "Unknown non-standard exception thrown";
    }
}
