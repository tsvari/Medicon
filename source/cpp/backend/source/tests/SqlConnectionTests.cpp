/**
 * @file SqlConnectionTests.cpp
 * @brief Unit tests for SqlConnection class
 * 
 * This file contains Google Test unit tests for the SqlConnection class,
 * verifying error handling, credential validation, move semantics,
 * and transaction management. No global state.
 * 
 * @version 3.0
 * @date 2026-08-02
 */

#include "sqlconnection.h"
#include "gtest/gtest.h"

/**
 * @class SqlConnectionTest
 * @brief Test fixture for SqlConnection tests
 * 
 * No setup needed — no global state.
 */
class SqlConnectionTest : public ::testing::Test {
protected:
};

/**
 * @test Verify parameterized constructor with valid credentials
 * 
 * Tests the recommended way of creating connections with explicit credentials.
 */
TEST_F(SqlConnectionTest, ParameterizedConstructor_ValidParams_CreatesObject)
{
    // Create connection with explicit credentials (should not throw)
    EXPECT_NO_THROW({
        SqlConnection connection(SA_Client_NotSpecified, "localhost", "user", "password");
    });
}

/**
 * @test Verify parameterized constructor rejects invalid credentials
 * 
 * Ensures validation catches empty or null parameters.
 */
TEST_F(SqlConnectionTest, ParameterizedConstructor_InvalidParams_ThrowsException)
{
    // Empty host
    EXPECT_THROW({
        SqlConnection connection(SA_Client_NotSpecified, "", "user", "pass");
    }, std::invalid_argument);
    
    // Empty user
    EXPECT_THROW({
        SqlConnection connection(SA_Client_NotSpecified, "host", "", "pass");
    }, std::invalid_argument);
    
    // Empty password
    EXPECT_THROW({
        SqlConnection connection(SA_Client_NotSpecified, "host", "user", "");
    }, std::invalid_argument);
    
    // Null parameters
    EXPECT_THROW({
        SqlConnection connection(SA_Client_NotSpecified, nullptr, "user", "pass");
    }, std::invalid_argument);
}

/**
 * @test Verify connection info string generation
 * 
 * Tests that getConnectionInfo produces expected format without exposing password.
 */
TEST_F(SqlConnectionTest, GetConnectionInfo_ReturnsFormattedString)
{
    SqlConnection connection(SA_Client_NotSpecified, "testhost", "testuser", "secretpass");
    
    std::string info = connection.getConnectionInfo();
    
    // Should contain user and host
    EXPECT_NE(info.find("testuser"), std::string::npos);
    EXPECT_NE(info.find("testhost"), std::string::npos);
    
    // Should NOT contain password
    EXPECT_EQ(info.find("secretpass"), std::string::npos);
    
    // Should contain connection status
    EXPECT_NE(info.find("disconnected"), std::string::npos);
}

/**
 * @test Verify move constructor transfers ownership
 * 
 * Tests that connection can be moved without copying.
 */
TEST_F(SqlConnectionTest, MoveConstructor_TransfersOwnership)
{
    SqlConnection conn1(SA_Client_NotSpecified, "localhost", "user", "pass");
    
    // Move construct conn2 from conn1
    SqlConnection conn2(std::move(conn1));
    
    // conn2 should have valid connection info
    std::string info = conn2.getConnectionInfo();
    EXPECT_NE(info.find("user"), std::string::npos);
    EXPECT_NE(info.find("localhost"), std::string::npos);
}

/**
 * @test Verify move assignment transfers ownership
 * 
 * Tests that connection can be move-assigned.
 */
TEST_F(SqlConnectionTest, MoveAssignment_TransfersOwnership)
{
    SqlConnection conn1(SA_Client_NotSpecified, "server1", "user1", "pass1");
    SqlConnection conn2(SA_Client_NotSpecified, "server2", "user2", "pass2");
    
    // Move assign conn1 to conn2
    conn2 = std::move(conn1);
    
    // conn2 should now have conn1's credentials
    std::string info = conn2.getConnectionInfo();
    EXPECT_NE(info.find("user1"), std::string::npos);
    EXPECT_NE(info.find("server1"), std::string::npos);
}

/**
 * @test Verify isConnected returns false when not connected
 * 
 * Tests initial connection state.
 */
TEST_F(SqlConnectionTest, IsConnected_InitialState_ReturnsFalse)
{
    SqlConnection connection(SA_Client_NotSpecified, "localhost", "user", "pass");
    
    // Should not be connected initially
    EXPECT_FALSE(connection.isConnected());
}

/**
 * @test Verify commit/rollback throw when not connected
 * 
 * Ensures operations requiring connection fail gracefully.
 */
TEST_F(SqlConnectionTest, TransactionOperations_WhenNotConnected_ThrowException)
{
    SqlConnection connection(SA_Client_NotSpecified, "localhost", "user", "pass");
    
    // commit() should throw when not connected
    EXPECT_THROW({
        connection.commit();
    }, std::runtime_error);
    
    // rollback() should throw when not connected
    EXPECT_THROW({
        connection.rollback();
    }, std::runtime_error);
}

/**
 * @test Verify connectionSa returns non-null pointer
 * 
 * Tests access to underlying SQLAPI++ connection object.
 */
TEST_F(SqlConnectionTest, ConnectionSa_ReturnsValidPointer)
{
    SqlConnection connection(SA_Client_NotSpecified, "localhost", "user", "pass");
    
    // Should return valid pointer
    EXPECT_NE(connection.connectionSa(), nullptr);
    
    // Const version should also work
    const SqlConnection& const_ref = connection;
    EXPECT_NE(const_ref.connectionSa(), nullptr);
}

/**
 * @test Verify disconnect is safe to call when not connected
 * 
 * Tests that disconnect() is idempotent.
 */
TEST_F(SqlConnectionTest, Disconnect_WhenNotConnected_NoException)
{
    SqlConnection connection(SA_Client_NotSpecified, "localhost", "user", "pass");
    
    // Calling disconnect when not connected should not throw
    EXPECT_NO_THROW({
        connection.disconnect();
    });
}

/*
 * TODO: Integration tests to implement (require actual database):
 * 
 * - Test connect() with valid database credentials
 * - Test connect() with invalid credentials throws SAException
 * - Test reconnection when already connected
 * - Test commit() persists changes to database
 * - Test rollback() discards changes
 * - Test setAutoCommit(true) auto-commits statements
 * - Test setAutoCommit(false) requires explicit commit
 * - Test destructor properly closes active connection
 * - Test exception handling during connection failures
 * - Test transaction isolation levels
 * - Test connection pooling scenarios
 * - Test concurrent connections from multiple threads
 * - Test connection timeout behavior
 * - Test handling of network interruptions
 */

/**
 * Example integration test structure (commented out - requires database):
 */
/*
TEST_F(SqlConnectionTest, DISABLED_Integration_Connect_ValidCredentials_Succeeds)
{
    // Configure with actual database credentials
    const char* host = "localhost:3306";
    const char* user = "test_user";
    const char* pass = "test_password";
    
    SqlConnection connection(SA_MySQL_Client, host, user, pass);
    
    // Should connect successfully
    ASSERT_NO_THROW({
        connection.connect();
    });
    
    // Should be connected
    EXPECT_TRUE(connection.isConnected());
    
    // Test basic query
    try {
        SACommand cmd(connection.connectionSa(), _TSA("SELECT 1"));
        cmd.Execute();
        
        // Verify result
        cmd.FetchNext();
        EXPECT_EQ(cmd.Field(1).asLong(), 1);
        
    } catch (const SAException& e) {
        FAIL() << "Query failed: " << e.ErrText().GetMultiByteChars();
    }
    
    // Cleanup
    connection.disconnect();
    EXPECT_FALSE(connection.isConnected());
}
*/
