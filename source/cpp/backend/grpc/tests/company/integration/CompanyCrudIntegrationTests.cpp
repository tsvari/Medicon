/**
 * @file CompanyCrudIntegrationTests.cpp
 * @brief Integration tests for Company CRUD operations using SQLite
 *
 * Tests the full SQL template lifecycle for company operations:
 * 1. Load company .sql files via SqlTemplate
 * 2. Verify SQL generation (named params, debug output)
 * 3. Execute CRUD against SQLite in-memory using SqlDirectCommand
 *    (limited by SQLAPI++ SQLite driver: named param binding unsupported)
 *
 * For true end-to-end CompanyRepository testing, run against PostgreSQL
 * (see CompanyRepositoryPostgresTests if available).
 */

#include "sqltemplate.h"
#include "sqlcommand.h"
#include "sqlconnection.h"
#include "column_allowlist.h"
#include "configfile.h"
#include "gtest/gtest.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// ============================================================================
// Test fixture
// ============================================================================

class CompanyCrudIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Determine applet path from project config
        // Provider's app-data holds the company .sql templates
        ConfigFile cfg(ALL_PROJECT_APPDATA_PATH, "provider");
        ASSERT_NO_THROW(cfg.load());
        m_appletPath = cfg.appletPath();

        // Validate directory exists
        ASSERT_TRUE(fs::is_directory(m_appletPath))
            << "Applet directory not found: " << m_appletPath;

        // Register COMPANY_COLUMNS for $FILTER_FIELD validation
        static constexpr auto COMPANY_COLUMNS = ColumnAllowList<9>({
            "UID", "SERVER_UID", "COMPANY_TYPE", "NAME",
            "ADDRESS", "REG_DATE", "JOINT_DATE", "LICENSE", "LOGO"
        });
        m_companyColumns = &COMPANY_COLUMNS;
    }

    std::string m_appletPath;
    const ColumnAllowListBase* m_companyColumns = nullptr;
};

// ============================================================================
// SQL Template Generation Tests
// ============================================================================

/**
 * @test company_insert.sql generates correct parameterized SQL and debug output
 */
TEST_F(CompanyCrudIntegrationTest, InsertTemplate_GeneratesCorrectSql)
{
    SqlTemplate tpl(m_appletPath + "company_insert.sql");
    tpl.addParameter("SERVER_UID", 1);
    tpl.addParameter("COMPANY_TYPE", 2);
    tpl.addParameter("NAME", "TestCorp");
    tpl.addParameter("ADDRESS", "123 Main St");
    tpl.addParameter("REG_DATE", "2020-01-15", DataInfo::Date);
    tpl.addParameter("JOINT_DATE", "2020-06-20", DataInfo::Date);
    tpl.addParameter("LICENSE", "LIC-001");
    tpl.addParameter("LOGO", "");

    ASSERT_NO_THROW(tpl.parse());

    // Verify :name markers in output SQL
    std::string sql = tpl.sql();
    EXPECT_NE(sql.find(":SERVER_UID"), std::string::npos);
    EXPECT_NE(sql.find(":NAME"), std::string::npos);
    EXPECT_NE(sql.find(":ADDRESS"), std::string::npos);

    // Verify debug SQL has substituted values
    std::string debug = tpl.getDebugSql();
    EXPECT_NE(debug.find("TestCorp"), std::string::npos);
    EXPECT_NE(debug.find("123 Main St"), std::string::npos);
    EXPECT_NE(debug.find("LIC-001"), std::string::npos);
}

/**
 * @test company_select.sql generates correct SQL with $FILTER_FIELD
 */
TEST_F(CompanyCrudIntegrationTest, SelectTemplate_GeneratesCorrectSql)
{
    SqlTemplate tpl(m_appletPath + "company_select.sql");
    tpl.addParameter("SERVER_UID", 1);
    tpl.addParameter("FILTER_VALUE", "Test");
    tpl.addParameter("FILTER_FIELD", "NAME");
    tpl.addParameter("OFFSET", 0);
    tpl.addParameter("LIMIT", 50);
    tpl.setColumnValidator("FILTER_FIELD", m_companyColumns);

    ASSERT_NO_THROW(tpl.parse());

    std::string sql = tpl.sql();
    EXPECT_NE(sql.find(":SERVER_UID"), std::string::npos);
    EXPECT_NE(sql.find(":FILTER_VALUE"), std::string::npos);
    EXPECT_NE(sql.find(":OFFSET"), std::string::npos);
    EXPECT_NE(sql.find(":LIMIT"), std::string::npos);

    // $FILTER_FIELD should be inlined with quotes as column name
    std::string debug = tpl.getDebugSql();
    EXPECT_NE(debug.find("\"NAME\""), std::string::npos);
}

/**
 * @test company_update.sql generates correct parameterized SQL
 */
TEST_F(CompanyCrudIntegrationTest, UpdateTemplate_GeneratesCorrectSql)
{
    SqlTemplate tpl(m_appletPath + "company_update.sql");
    tpl.addParameter("UID", "test-uid-123");
    tpl.addParameter("SERVER_UID", 1);
    tpl.addParameter("COMPANY_TYPE", 2);
    tpl.addParameter("NAME", "UpdatedCorp");
    tpl.addParameter("ADDRESS", "456 Oak Ave");
    tpl.addParameter("REG_DATE", "2021-03-10", DataInfo::Date);
    tpl.addParameter("JOINT_DATE", "2021-08-15", DataInfo::Date);
    tpl.addParameter("LICENSE", "LIC-002");
    tpl.addParameter("LOGO", "");

    ASSERT_NO_THROW(tpl.parse());

    std::string debug = tpl.getDebugSql();
    EXPECT_NE(debug.find("UpdatedCorp"), std::string::npos);
    EXPECT_NE(debug.find("456 Oak Ave"), std::string::npos);
    EXPECT_NE(debug.find("test-uid-123"), std::string::npos);
}

/**
 * @test company_delete.sql generates correct parameterized SQL
 */
TEST_F(CompanyCrudIntegrationTest, DeleteTemplate_GeneratesCorrectSql)
{
    SqlTemplate tpl(m_appletPath + "company_delete.sql");
    tpl.addParameter("UID", "uid-to-delete");

    ASSERT_NO_THROW(tpl.parse());

    std::string debug = tpl.getDebugSql();
    EXPECT_NE(debug.find("uid-to-delete"), std::string::npos);
}

// ============================================================================
// Database CRUD Tests (SQLite in-memory)
//
// Note: These tests use SqlDirectCommand (raw SQL) to avoid SQLAPI++ SQLite
// driver limitations with named parameter binding. The SQL templates are
// validated separately above — this section proves the SQL operations
// themselves work correctly against a real database.
// ============================================================================

/**
 * @test Create company table and insert a row using raw SQL
 */
TEST_F(CompanyCrudIntegrationTest, CreateTableAndInsert)
{
    // Skip if MEDICON_SKIP_DB_TESTS is set
    const char* skipEnv = std::getenv("MEDICON_SKIP_DB_TESTS");
    if (skipEnv && skipEnv[0] != '\0') {
        GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
    }

    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    ASSERT_NO_THROW(conn.connect());

    // Create company table
    SqlDirectCommand create(conn, SAString(
        "CREATE TABLE company ("
        "  \"UID\" TEXT PRIMARY KEY,"
        "  \"SERVER_UID\" INTEGER,"
        "  \"COMPANY_TYPE\" INTEGER,"
        "  \"NAME\" TEXT,"
        "  \"ADDRESS\" TEXT,"
        "  \"REG_DATE\" TEXT,"
        "  \"JOINT_DATE\" TEXT,"
        "  \"LICENSE\" TEXT,"
        "  \"LOGO\" BLOB"
        ")"
    ));
    ASSERT_NO_THROW(create.execute());

    // Insert a row using raw SQL (mimicking what company_insert.sql does)
    SqlDirectCommand insert(conn, SAString(
        "INSERT INTO company(\"UID\", \"SERVER_UID\", \"COMPANY_TYPE\", \"NAME\","
        " \"ADDRESS\", \"REG_DATE\", \"JOINT_DATE\", \"LICENSE\", \"LOGO\")"
        " VALUES('test-uid-1', 1, 2, 'TestCorp', '123 Main St',"
        " '2020-01-15', '2020-06-20', 'LIC-001', NULL)"
    ));
    ASSERT_NO_THROW(insert.execute());

    // Verify by selecting
    SqlDirectCommand select(conn, SAString("SELECT \"NAME\", \"SERVER_UID\" FROM company WHERE \"UID\" = 'test-uid-1'"));
    select.execute();
    ASSERT_TRUE(select.FetchNext());
    EXPECT_STREQ(select.Field(1).asString().GetMultiByteChars(), "TestCorp");
    EXPECT_EQ(select.Field(2).asLong(), 1);
}

/**
 * @test Update a company row
 */
TEST_F(CompanyCrudIntegrationTest, UpdateRow)
{
    const char* skipEnv = std::getenv("MEDICON_SKIP_DB_TESTS");
    if (skipEnv && skipEnv[0] != '\0') {
        GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
    }

    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    ASSERT_NO_THROW(conn.connect());

    // Setup
    SqlDirectCommand create(conn, SAString(
        "CREATE TABLE company (\"UID\" TEXT PRIMARY KEY, \"NAME\" TEXT, \"ADDRESS\" TEXT)"
    ));
    create.execute();

    SqlDirectCommand insert(conn, SAString(
        "INSERT INTO company VALUES('uid-1', 'OldName', 'OldAddr')"
    ));
    insert.execute();

    // Update (mimicking company_update.sql)
    SqlDirectCommand update(conn, SAString(
        "UPDATE company SET \"NAME\" = 'UpdatedName', \"ADDRESS\" = 'NewAddr' WHERE \"UID\" = 'uid-1'"
    ));
    ASSERT_NO_THROW(update.execute());

    // Verify
    SqlDirectCommand select(conn, SAString("SELECT \"NAME\", \"ADDRESS\" FROM company WHERE \"UID\" = 'uid-1'"));
    select.execute();
    ASSERT_TRUE(select.FetchNext());
    EXPECT_STREQ(select.Field(1).asString().GetMultiByteChars(), "UpdatedName");
    EXPECT_STREQ(select.Field(2).asString().GetMultiByteChars(), "NewAddr");
}

/**
 * @test Delete a company row
 */
TEST_F(CompanyCrudIntegrationTest, DeleteRow)
{
    const char* skipEnv = std::getenv("MEDICON_SKIP_DB_TESTS");
    if (skipEnv && skipEnv[0] != '\0') {
        GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
    }

    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    ASSERT_NO_THROW(conn.connect());

    // Setup
    SqlDirectCommand create(conn, SAString(
        "CREATE TABLE company (\"UID\" TEXT PRIMARY KEY, \"NAME\" TEXT)"
    ));
    create.execute();

    SqlDirectCommand insert(conn, SAString(
        "INSERT INTO company VALUES('uid-to-delete', 'DeleteMe')"
    ));
    insert.execute();

    // Verify it exists
    SqlDirectCommand check(conn, SAString("SELECT COUNT(*) FROM company"));
    check.execute();
    ASSERT_TRUE(check.FetchNext());
    EXPECT_EQ(check.Field(1).asLong(), 1);

    // Delete (mimicking company_delete.sql)
    SqlDirectCommand del(conn, SAString("DELETE FROM company WHERE \"UID\" = 'uid-to-delete'"));
    ASSERT_NO_THROW(del.execute());

    // Verify gone
    SqlDirectCommand verify(conn, SAString("SELECT COUNT(*) FROM company"));
    verify.execute();
    ASSERT_TRUE(verify.FetchNext());
    EXPECT_EQ(verify.Field(1).asLong(), 0);
}
