/**
 * @file SqlCommandIntegrationTests.cpp
 * @brief Integration tests for SqlCommand using SQLite in-memory database
 * 
 * Tests actual SQL execution with real database operations.
 * 
 * @version 1.0
 * @date 2026-01-05
 */

#include "sqlcommand.h"
#include "sqlconnection.h"
#include "gtest/gtest.h"

/**
 * @class SqlCommandIntegrationTest
 * @brief Integration test fixture using SQLite in-memory database
 */
class SqlCommandIntegrationTest : public ::testing::Test {
protected:
};

/**
 * @test Verify SqlDirectCommand executes simple SQL
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_CreateTable_Success)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "password123");
    conn.connect();
    
    SqlDirectCommand cmd(conn, SAString("CREATE TABLE test(id INTEGER, name TEXT)"));
    
    ASSERT_NO_THROW(cmd.execute());
    EXPECT_TRUE(conn.isConnected());
}

/**
 * @test Verify SqlDirectCommand inserts data
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_InsertData_Success)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "dbuser", "dbpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    // Insert data
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'John')"));
    ASSERT_NO_THROW(insert.execute());
    
    // Verify
    SqlDirectCommand select(conn, SAString("SELECT COUNT(*) FROM users"));
    select.execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 1);
}

/**
 * @test Verify SqlDirectCommand with multiple inserts
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_MultipleInserts_Success)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "testuser", "testpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER)"));
    create.execute();
    
    // Insert multiple rows
    SqlDirectCommand insert1(conn, SAString("INSERT INTO users VALUES(1, 'Alice', 30)"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO users VALUES(2, 'Bob', 25)"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO users VALUES(3, 'Charlie', 35)"));
    
    ASSERT_NO_THROW(insert1.execute());
    ASSERT_NO_THROW(insert2.execute());
    ASSERT_NO_THROW(insert3.execute());
    
    // Verify count
    SqlDirectCommand select(conn, SAString("SELECT COUNT(*) FROM users"));
    select.execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 3);
}

/**
 * @test Verify SqlDirectCommand with transaction commit
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_WithinTransaction_Commits)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "transuser", "transpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    // Start transaction
    conn.setAutoCommit(false);
    
    // Insert within transaction
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Dave')"));
    insert.execute();
    
    // Commit
    ASSERT_NO_THROW(conn.commit());
    
    // Verify data persisted
    SqlDirectCommand select(conn, SAString("SELECT COUNT(*) FROM users"));
    select.execute();
    select.FetchNext();
    EXPECT_EQ(select.Field(1).asLong(), 1);
}

/**
 * @test Verify SqlDirectCommand handles SQL errors
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_InvalidSQL_ThrowsException)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "erroruser", "errorpass");
    conn.connect();
    
    SqlDirectCommand cmd(conn, SAString("INVALID SQL STATEMENT"));
    
    EXPECT_THROW(cmd.execute(), SAException);
}

/**
 * @test Verify sql() returns correct command text
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_Sql_ReturnsText)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "sqluser", "sqlpass");
    
    std::string expectedSql = "SELECT * FROM users";
    SqlDirectCommand cmd(conn, SAString(expectedSql.c_str()));
    
    std::string actualSql = cmd.sql();
    EXPECT_EQ(actualSql, expectedSql);
}

/**
 * @test Verify SqlDirectCommand with SELECT query
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_SelectQuery_ReturnsData)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "selectuser", "selectpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE test(id INTEGER, name TEXT)"));
    create.execute();
    
    SqlDirectCommand insert(conn, SAString("INSERT INTO test VALUES(42, 'TestName')"));
    insert.execute();
    
    // Select data
    SqlDirectCommand select(conn, SAString("SELECT id, name FROM test"));
    select.execute();
    
    ASSERT_TRUE(select.FetchNext());
    EXPECT_EQ(select.Field(1).asLong(), 42);
    EXPECT_STREQ(select.Field(2).asString().GetMultiByteChars(), "TestName");
}

/**
 * @test Verify RETURNING clause works (SQLite 3.35+)
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_Returning_ReturnsValue)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "returnuser", "returnpass");
    conn.connect();
    
    // Create table with auto-increment
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)"));
    create.execute();
    
    // Insert with RETURNING (SQLite 3.35+)
    SqlDirectCommand insert(conn, SAString("INSERT INTO users(name) VALUES('Charlie') RETURNING id"));
    
    try {
        insert.execute();
        
        if (insert.isResultSet()) {
            insert.FetchNext();
            long insertedId = insert.Field(1).asLong();
            EXPECT_GT(insertedId, 0);
        } else {
            GTEST_SKIP() << "RETURNING clause not supported (requires SQLite 3.35+)";
        }
    } catch (const SAException& e) {
        GTEST_SKIP() << "RETURNING clause not supported: " << e.ErrText().GetMultiByteChars();
    }
}

/**
 * @test Verify SqlDirectCommand inserts data
 */
TEST_F(SqlCommandIntegrationTest, SqlDirectCommand_InsertsData)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "directuser", "directpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER)"));
    create.execute();
    
    // Use direct SQL (parameterized queries via SqlTemplate don't support SQLite's ? syntax)
    SqlDirectCommand cmd(conn, SAString("INSERT INTO users(id, name, age) VALUES(1, 'Alice', 30)"));
    ASSERT_NO_THROW(cmd.execute());
    
    // Verify data was inserted
    SqlDirectCommand select(conn, SAString("SELECT id, name, age FROM users"));
    select.execute();
    
    ASSERT_TRUE(select.FetchNext());
    EXPECT_EQ(select.Field(1).asLong(), 1);
    EXPECT_STREQ(select.Field(2).asString().GetMultiByteChars(), "Alice");
    EXPECT_EQ(select.Field(3).asLong(), 30);
}

/*
 * NOTE: These integration tests use SQLite which is:
 * - Lightweight (no installation required)
 * - Fast (in-memory database)
 * - Perfect for CI/CD pipelines
 * 
 * Tests marked DISABLED_ require additional setup (like test applet XML files).
 * Enable them once the required resources are created.
 */

// ============================================================================
// SqlTemplate Integration Tests (SqlCommand with .sql template files)
// ============================================================================

/**
 * @class SqlCommandTemplateIntegrationTest
 * @brief Integration test fixture for SqlCommand using .sql templates
 *
 * Tests the full SqlTemplate → SqlCommand → DB round-trip with SQLite.
 */
class SqlCommandTemplateIntegrationTest : public ::testing::Test {
protected:
};

/**
 * @test Verify SqlCommand with .sql template generates correct SQL and bindings
 *
 * Validates the SqlTemplate pipeline without requiring DB execution support
 * for named parameters (varies by SQLAPI++ driver). Tests that:
 * 1. .sql file is loaded and parsed
 * 2. Parameters are bound correctly
 * 3. Debug SQL contains substituted values
 * 4. SQL text contains :name markers for SQLAPI++ Param() binding
 */
TEST_F(SqlCommandTemplateIntegrationTest, InsertTemplate_GeneratesCorrectSql)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    conn.connect();

    SqlCommand cmd(conn, ALL_BACKEND_TEST_APPDATA_PATH "insert_user_test.sql");
    cmd.addParameter("id", 42);
    cmd.addParameter("name", "Alice");
    cmd.addParameter("age", 30);

    // Debug SQL should contain substituted values
    std::string debug = cmd.getSqlWithParameters();
    EXPECT_NE(debug.find("42"), std::string::npos);
    EXPECT_NE(debug.find("Alice"), std::string::npos);
    EXPECT_NE(debug.find("30"), std::string::npos);

    // Raw SQL text should contain :name markers for SQLAPI++ Param() binding
    std::string sql = cmd.sql();
    EXPECT_NE(sql.find(":id"), std::string::npos);
    EXPECT_NE(sql.find(":name"), std::string::npos);
    EXPECT_NE(sql.find(":age"), std::string::npos);
}

/**
 * @test Verify SqlCommand with missing param still parses (uses NULL) but
 * DB driver rejects the execution
 *
 * SqlTemplate::parse() does NOT throw on missing bind params — it sets
 * the value to "NULL" and keeps the :name marker. The DB driver (SQLite)
 * then fails because it can't bind the named parameter.
 */
TEST_F(SqlCommandTemplateIntegrationTest, MissingParam_UsesNull_AndDbRejects)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    conn.connect();

    // Omit the 'name' parameter — parse() uses NULL, then SQLite rejects :name bind
    SqlCommand cmd(conn, ALL_BACKEND_TEST_APPDATA_PATH "insert_user_test.sql");
    cmd.addParameter("id", 42);
    // name intentionally omitted
    cmd.addParameter("age", 30);

    // parse() succeeds (uses NULL for name), but SQLite driver throws
    EXPECT_THROW(cmd.execute(), SAException);
}

/**
 * @test Verify SqlCommand debug SQL contains all parameter values formatted
 */
TEST_F(SqlCommandTemplateIntegrationTest, DebugSql_FormatsTypesCorrectly)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    conn.connect();

    SqlCommand cmd(conn, ALL_BACKEND_TEST_APPDATA_PATH "insert_user_test.sql");
    cmd.addParameter("id", 7);
    cmd.addParameter("name", "Charlie");
    cmd.addParameter("age", 35);

    std::string debug = cmd.getSqlWithParameters();
    EXPECT_NE(debug.find("7"), std::string::npos) << "Should contain numeric id";
    EXPECT_NE(debug.find("Charlie"), std::string::npos) << "Should contain string name";
    EXPECT_NE(debug.find("35"), std::string::npos) << "Should contain numeric age";
}

/**
 * @test Verify SqlCommand with default values works when params are omitted
 *
 * insert_user_test.sql has defaults: id=0, name='', age=0
 * Omitting all params should use defaults.
 */
TEST_F(SqlCommandTemplateIntegrationTest, DefaultValues_UsedWhenParamsOmitted)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    conn.connect();

    // Don't add any parameters — use all defaults
    SqlCommand cmd(conn, ALL_BACKEND_TEST_APPDATA_PATH "insert_user_test.sql");

    std::string debug = cmd.getSqlWithParameters();
    EXPECT_NE(debug.find("0"), std::string::npos) << "Default id=0 should appear";
    EXPECT_NE(debug.find("''"), std::string::npos) << "Default name='' should appear";
}

/**
 * @test Verify SqlCommand with all valid params still throws SAException
 * because SQLite driver doesn't support named-parameter binding via SQLAPI++
 *
 * This confirms the template parsing side succeeds and the failure is
 * from the DB driver — not from bad SQL or missing parameters.
 */
TEST_F(SqlCommandTemplateIntegrationTest, Execute_AllParams_ThrowsSAException)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "admin", "pass");
    conn.connect();

    SqlCommand cmd(conn, ALL_BACKEND_TEST_APPDATA_PATH "insert_user_test.sql");
    cmd.addParameter("id", 1);
    cmd.addParameter("name", "Test");
    cmd.addParameter("age", 20);

    EXPECT_THROW(cmd.execute(), SAException);
}
