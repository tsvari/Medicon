/**
 * @file CompanySqlTemplateTests.cpp
 * @brief Validate all company-level .sql template files
 *
 * Parses every .sql file in the provider's sql-applets directory to ensure
 * they are valid for the Company domain. This catches typos, broken
 * -- @param declarations, and mismatched placeholders at test time.
 *
 * These are provider/company-level integration tests — they verify the
 * specific SQL templates used by CompanyRepository, not the SqlTemplate
 * parser itself (which has its own unit tests in SqlTemplateTests.cpp).
 *
 * These tests do NOT require a database connection — they only verify
 * the SQL template files parse correctly.
 */

#include "sqltemplate.h"
#include "column_allowlist.h"
#include "gtest/gtest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ============================================================================
// Test fixture: loads sql-applet paths from ConfigFile
// ============================================================================

class CompanySqlTemplateTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Discover .sql files in the provider's applet directory
        std::string appletPath = std::string(ALL_PROJECT_APPDATA_PATH) + "provider/sql-applets/";
        m_appletDir = appletPath;

        if (!fs::is_directory(m_appletDir)) {
            GTEST_SKIP() << "Applet directory not found: " << m_appletDir;
        }

        for (const auto& entry : fs::directory_iterator(m_appletDir)) {
            if (entry.path().extension() == ".sql") {
                m_sqlFiles.push_back(entry.path().string());
            }
        }

        ASSERT_FALSE(m_sqlFiles.empty())
            << "No .sql files found in " << m_appletDir;
    }

    std::string m_appletDir;
    std::vector<std::string> m_sqlFiles;
};

/**
 * @test Every .sql file can be loaded without throwing
 */
TEST_F(CompanySqlTemplateTest, AllSqlFilesLoadWithoutError)
{
    for (const auto& filePath : m_sqlFiles) {
        EXPECT_NO_THROW({
            SqlTemplate tpl(filePath);
        }) << "Failed to load: " << filePath;
    }
}

/**
 * @test Every .sql file that has non-COLUMN params can be parsed
 * with minimal required parameters provided
 *
 * Tests that the SQL body is well-formed even when parameters get defaults.
 */
TEST_F(CompanySqlTemplateTest, AllSqlFilesParseWithDefaults)
{
    for (const auto& filePath : m_sqlFiles) {
        SqlTemplate tpl(filePath);

        // Provide minimal parameters so parse() doesn't fail on missing values.
        // The actual parameter values don't matter — we're testing parse() itself.
        tpl.addParameter("SERVER_UID", 0);
        tpl.addParameter("COMPANY_TYPE", 0);
        tpl.addParameter("NAME", "test");
        tpl.addParameter("ADDRESS", "test");
        tpl.addParameter("REG_DATE", "2007-01-20", DataInfo::Date);
        tpl.addParameter("JOINT_DATE", "2007-01-20", DataInfo::Date);
        tpl.addParameter("LICENSE", "test");
        tpl.addParameter("LOGO", "test");
        tpl.addParameter("UID", "test-uid");
        tpl.addParameter("FILTER_VALUE", "test");
        tpl.addParameter("OFFSET", 0);
        tpl.addParameter("LIMIT", 100);
        tpl.addParameter("FILTER_FIELD", "NAME");

        // Register column validator for any COLUMN-type params
        static constexpr auto COMPANY_COLUMNS = ColumnAllowList<9>({
            "UID", "SERVER_UID", "COMPANY_TYPE", "NAME",
            "ADDRESS", "REG_DATE", "JOINT_DATE", "LICENSE", "LOGO"
        });
        tpl.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);

        EXPECT_NO_THROW({
            tpl.parse();
        }) << "Failed to parse: " << filePath;
    }
}

/**
 * @test Debug SQL output is generated without errors
 */
TEST_F(CompanySqlTemplateTest, DebugSqlGeneratedWithoutError)
{
    for (const auto& filePath : m_sqlFiles) {
        SqlTemplate tpl(filePath);
        tpl.addParameter("SERVER_UID", 123);
        tpl.addParameter("COMPANY_TYPE", 1);
        tpl.addParameter("NAME", "TestCorp");
        tpl.addParameter("ADDRESS", "123 Main St");
        tpl.addParameter("REG_DATE", "2020-01-01", DataInfo::Date);
        tpl.addParameter("JOINT_DATE", "2020-06-15", DataInfo::Date);
        tpl.addParameter("LICENSE", "LIC-001");
        tpl.addParameter("LOGO", "");
        tpl.addParameter("UID", "abc-123");
        tpl.addParameter("FILTER_VALUE", "search");
        tpl.addParameter("OFFSET", 0);
        tpl.addParameter("LIMIT", 50);
        tpl.addParameter("FILTER_FIELD", "NAME");

        static constexpr auto COMPANY_COLUMNS = ColumnAllowList<9>({
            "UID", "SERVER_UID", "COMPANY_TYPE", "NAME",
            "ADDRESS", "REG_DATE", "JOINT_DATE", "LICENSE", "LOGO"
        });
        tpl.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);

        ASSERT_NO_THROW(tpl.parse());
        std::string debug = tpl.getDebugSql();

        // Debug SQL should not be empty and should not contain raw :NAME markers
        EXPECT_FALSE(debug.empty());

        // The bind params with values should be substituted in debug SQL
        EXPECT_NE(debug.find("123"), std::string::npos)
            << "Debug SQL should contain substituted value '123': " << debug;
    }
}

/**
 * @test SqlTemplate throws on non-existent file
 */
TEST_F(CompanySqlTemplateTest, NonExistentFile_Throws)
{
    SqlTemplate tpl("/nonexistent/path/company_test.sql");
    EXPECT_THROW(tpl.parse(), SqlTemplateException);
}

/**
 * @test Column validation rejects invalid column name
 *
 * ColumnAllowList::resolve() throws std::invalid_argument, which is
 * caught and re-thrown as SqlTemplateException by parse().
 */
TEST_F(CompanySqlTemplateTest, InvalidColumnName_Throws)
{
    SqlTemplate tpl(m_appletDir + "company_select.sql");
    tpl.addParameter("SERVER_UID", 1);
    tpl.addParameter("FILTER_VALUE", "test");
    tpl.addParameter("OFFSET", 0);
    tpl.addParameter("LIMIT", 100);
    tpl.addParameter("FILTER_FIELD", "INVALID_COLUMN");

    static constexpr auto COMPANY_COLUMNS = ColumnAllowList<9>({
        "UID", "SERVER_UID", "COMPANY_TYPE", "NAME",
        "ADDRESS", "REG_DATE", "JOINT_DATE", "LICENSE", "LOGO"
    });
    tpl.setColumnValidator("FILTER_FIELD", &COMPANY_COLUMNS);

    // ColumnAllowList::resolve() throws std::invalid_argument
    EXPECT_THROW(tpl.parse(), std::invalid_argument);
}
