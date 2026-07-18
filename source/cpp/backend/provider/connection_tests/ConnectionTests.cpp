/**
 * @file ConnectionTests.cpp
 * @brief Connection integration tests for ConfigFile and SqlConnection
 *
 * Tests that ConfigFile loads provider configuration correctly and that
 * SqlConnection can be parameterized properly. The actual DB connection
 * test is conditional — it only runs if PostgreSQL is available.
 *
 * Uses the RECOMMENDED parameterized SqlConnection constructor pattern.
 * No global InitAllConnections() required.
 */

#include "configfile.h"
#include "gtest/gtest.h"
#include "sqlconnection.h"

#include <cstdlib>

// ============================================================================
// ConfigFile Integration Tests
// ============================================================================

/**
 * @test Verify ConfigFile loads provider config from real app-data paths
 */
TEST(ConnectionIntegrationTests, ConfigFileLoadsCorrectly)
{
    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    ASSERT_NO_THROW(config.load());

    // Validate expected config keys exist (from provider.json)
    EXPECT_NO_THROW(config.value("host"));
    EXPECT_NO_THROW(config.value("user"));
    EXPECT_NO_THROW(config.value("pass"));

    // Non-existent key should throw
    EXPECT_THROW(config.value("NonExistentKey"), std::out_of_range);

    // Paths should be populated
    EXPECT_FALSE(config.appletPath().empty());
    EXPECT_FALSE(config.logFilePath().empty());
    EXPECT_FALSE(config.projectPath().empty());
}

/**
 * @test Verify ConfigFile path resolution matches expected app-data layout
 */
TEST(ConnectionIntegrationTests, ConfigFilePathResolution)
{
    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    ASSERT_NO_THROW(config.load());

    std::string expectedPrefix = std::string(ALL_PROJECT_APPDATA_PATH) + "provider/";
    EXPECT_EQ(config.appletPath(), expectedPrefix + "sql-applets/");
    EXPECT_EQ(config.logFilePath(), expectedPrefix + "log/provider.log");
    EXPECT_EQ(config.projectPath(), expectedPrefix);
}

// ============================================================================
// SqlConnection Construction Tests
// ============================================================================

/**
 * @test Verify parameterized SqlConnection can be constructed (no DB needed)
 */
TEST(ConnectionIntegrationTests, SqlConnectionParameterizedConstructor)
{
    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    ASSERT_NO_THROW(config.load());

    // Constructing should not throw — no actual connection yet
    SqlConnection conn(SA_PostgreSQL_Client,
                       config.value("host").c_str(),
                       config.value("user").c_str(),
                       config.value("pass").c_str());

    // Connection info should contain user and host, but NOT password
    std::string info = conn.getConnectionInfo();
    EXPECT_NE(info.find(config.value("user")), std::string::npos);
    EXPECT_NE(info.find(config.value("host")), std::string::npos);
    EXPECT_EQ(info.find(config.value("pass")), std::string::npos);
    EXPECT_NE(info.find("disconnected"), std::string::npos);
}

/**
 * @test Verify SqlConnection rejects empty credentials
 */
TEST(ConnectionIntegrationTests, SqlConnectionInvalidCredentials_Throws)
{
    EXPECT_THROW(SqlConnection(SA_Client_NotSpecified, "", "user", "pass"),
                 std::invalid_argument);
    EXPECT_THROW(SqlConnection(SA_Client_NotSpecified, "host", "", "pass"),
                 std::invalid_argument);
    EXPECT_THROW(SqlConnection(SA_Client_NotSpecified, "host", "user", ""),
                 std::invalid_argument);
    EXPECT_THROW(SqlConnection(SA_Client_NotSpecified, nullptr, "user", "pass"),
                 std::invalid_argument);
}

// ============================================================================
// Actual Database Connection Test
// ============================================================================

/**
 * @test Try to connect to PostgreSQL using provider config
 *
 * This test SKIPS if the environment variable MEDICON_SKIP_DB_TESTS is set
 * or if no PostgreSQL is available. This allows CI to run without a DB.
 */
TEST(ConnectionIntegrationTests, TryConnectToPostgreSQL)
{
    // Allow skipping DB-dependent tests
    const char* skipEnv = std::getenv("MEDICON_SKIP_DB_TESTS");
    if (skipEnv && skipEnv[0] != '\0') {
        GTEST_SKIP() << "Skipping DB test: MEDICON_SKIP_DB_TESTS is set";
    }

    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    ASSERT_NO_THROW(config.load());

    SqlConnection conn(SA_PostgreSQL_Client,
                       config.value("host").c_str(),
                       config.value("user").c_str(),
                       config.value("pass").c_str());

    try {
        conn.connect();
        EXPECT_TRUE(conn.isConnected());
        SUCCEED() << "Successfully connected to PostgreSQL";
        conn.disconnect();
    } catch (const SAException& e) {
        // Connection failed — skip, not fail (DB might not be running)
        GTEST_SKIP() << "PostgreSQL not available: " << e.ErrText().GetMultiByteChars();
    }
}





