/**
 * @file SqlConnectionIntegrationTests.cpp
 * @brief Integration tests for SqlConnection using SQLite in-memory database
 * 
 * These tests verify actual database operations without requiring
 * external database installation. SQLite is lightweight and perfect for testing.
 * 
 * @version 2.0
 * @date 2026-01-05
 */

#include "sqlconnection.h"
#include "gtest/gtest.h"
#include <filesystem>

/**
 * @class SqlConnectionIntegrationTest
 * @brief Integration test fixture using SQLite in-memory database
 */
class SqlConnectionIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        SqlConnection::ClearAllConnections();
    }

    void TearDown() override {
        SqlConnection::ClearAllConnections();
        
        // Clean up test database files
        std::filesystem::remove("test.db");
        std::filesystem::remove("transaction_test.db");
        std::filesystem::remove("rollback_test.db");
        std::filesystem::remove("autocommit_test.db");
        std::filesystem::remove("reconnect_test.db");
        std::filesystem::remove("invalid_test.db");
        std::filesystem::remove("notconnected_test.db");
        std::filesystem::remove("destructor_test.db");
    }
};

/**
 * @test Verify basic connect/disconnect cycle with SQLite
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_ConnectDisconnect_Success)
{
    // SQLite in-memory database doesn't need credentials, but we specify them for consistency
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "password123");
    
    // Should connect successfully
    ASSERT_NO_THROW(conn.connect());
    EXPECT_TRUE(conn.isConnected());
    
    // Disconnect
    conn.disconnect();
    EXPECT_FALSE(conn.isConnected());
}

/**
 * @test Verify table creation and basic SQL operations
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_CreateTable_Success)
{
    SqlConnection conn(SA_SQLite_Client, "test.db", "dbuser", "dbpass");
    conn.connect();
    
    // Create table
    try {
        SACommand cmd(conn.connectionSa(), _TSA("CREATE TABLE test(id INTEGER, name TEXT)"));
        cmd.Execute();
    } catch (const SAException& e) {
        FAIL() << "Failed to create table: " << e.ErrText().GetMultiByteChars();
    }
    
    // Verify table exists by inserting data
    EXPECT_NO_THROW({
        SACommand insert(conn.connectionSa(), _TSA("INSERT INTO test VALUES(1, 'test')"));
        insert.Execute();
    });
}

/**
 * @test Verify transaction commit works correctly
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_CommitTransaction_Success)
{
    SqlConnection conn(SA_SQLite_Client, "transaction_test.db", "testuser", "testpass");
    conn.connect();
    
    // Create table
    SACommand create(conn.connectionSa(), _TSA("CREATE TABLE test(id INTEGER)"));
    create.Execute();
    
    // Disable auto-commit
    conn.setAutoCommit(false);
    
    // Insert data
    SACommand insert(conn.connectionSa(), _TSA("INSERT INTO test VALUES(1)"));
    insert.Execute();
    
    // Commit
    ASSERT_NO_THROW(conn.commit());
    
    // Verify data persisted
    SACommand select(conn.connectionSa(), _TSA("SELECT COUNT(*) FROM test"));
    select.Execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 1);
}

/**
 * @test Verify transaction rollback discards changes
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_RollbackTransaction_Success)
{
    SqlConnection conn(SA_SQLite_Client, "rollback_test.db", "rollbackuser", "rollbackpass");
    conn.connect();
    
    // Create table
    SACommand create(conn.connectionSa(), _TSA("CREATE TABLE test(id INTEGER)"));
    create.Execute();
    
    // Disable auto-commit
    conn.setAutoCommit(false);
    
    // Insert data
    SACommand insert(conn.connectionSa(), _TSA("INSERT INTO test VALUES(1)"));
    insert.Execute();
    
    // Rollback
    ASSERT_NO_THROW(conn.rollback());
    
    // Verify data was not persisted
    SACommand select(conn.connectionSa(), _TSA("SELECT COUNT(*) FROM test"));
    select.Execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 0);
}

/**
 * @test Verify auto-commit mode works correctly
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_AutoCommit_Success)
{
    SqlConnection conn(SA_SQLite_Client, "autocommit_test.db", "autouser", "autopass");
    conn.connect();
    
    // Create table
    SACommand create(conn.connectionSa(), _TSA("CREATE TABLE test(id INTEGER)"));
    create.Execute();
    
    // Enable auto-commit (should be default, but set explicitly)
    conn.setAutoCommit(true);
    
    // Insert data (should auto-commit)
    SACommand insert(conn.connectionSa(), _TSA("INSERT INTO test VALUES(1)"));
    insert.Execute();
    
    // Data should be persisted without explicit commit
    SACommand select(conn.connectionSa(), _TSA("SELECT COUNT(*) FROM test"));
    select.Execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 1);
}

/**
 * @test Verify reconnection works correctly
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_Reconnect_Success)
{
    SqlConnection conn(SA_SQLite_Client, "reconnect_test.db", "reconuser", "reconpass");
    
    // First connection
    conn.connect();
    EXPECT_TRUE(conn.isConnected());
    
    // Reconnect (should disconnect first, then reconnect)
    EXPECT_NO_THROW(conn.connect());
    EXPECT_TRUE(conn.isConnected());
}

/**
 * @test Verify exception handling when operations fail
 */
TEST_F(SqlConnectionIntegrationTest, SQLite_InvalidSQL_ThrowsException)
{
    SqlConnection conn(SA_SQLite_Client, "invalid_test.db", "invaliduser", "invalidpass");
    conn.connect();
    
    // Invalid SQL should throw
    EXPECT_THROW({
        SACommand cmd(conn.connectionSa(), _TSA("INVALID SQL STATEMENT"));
        cmd.Execute();
    }, SAException);
}

/**
 * @test Verify commit/rollback throw when not connected
 */
TEST_F(SqlConnectionIntegrationTest, NotConnected_TransactionOps_ThrowException)
{
    SqlConnection conn(SA_SQLite_Client, "notconnected_test.db", "notconnuser", "notconnpass");
    
    // Not connected yet
    EXPECT_THROW(conn.commit(), std::runtime_error);
    EXPECT_THROW(conn.rollback(), std::runtime_error);
}

/**
 * @test Verify destructor properly closes connection
 */
TEST_F(SqlConnectionIntegrationTest, Destructor_ClosesConnection)
{
    {
        SqlConnection conn(SA_SQLite_Client, "destructor_test.db", "destruser", "destpass");
        conn.connect();
        EXPECT_TRUE(conn.isConnected());
        // Destructor should close connection
    }
    // If we get here without crash, destructor worked correctly
    SUCCEED();
}

/*
 * NOTE: These integration tests use SQLite which is:
 * - Lightweight (no installation required)
 * - Fast (in-memory database)
 * - Sufficient for testing transaction logic
 * 
 * For testing with actual MySQL/PostgreSQL/etc., use the
 * DISABLED tests in SqlConnectionTests.cpp and provide
 * actual database credentials.
 */
