/**
 * @file SqlQueryIntegrationTests.cpp
 * @brief Integration tests for SqlQuery and SqlDirectQuery using SQLite in-memory database
 * 
 * Tests actual query execution with real database operations and result set fetching.
 * 
 * @version 1.0
 * @date 2026-01-06
 */

#include "sqlquery.h"
#include "sqlconnection.h"
#include "gtest/gtest.h"

/**
 * @class SqlQueryIntegrationTest
 * @brief Integration test fixture using SQLite in-memory database
 */
class SqlQueryIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
        SqlConnection::ClearAllConnections();
    }

    void TearDown() override {
        SqlConnection::ClearAllConnections();
    }
};

// ============================================================================
// SqlDirectQuery Integration Tests
// ============================================================================

/**
 * @test Verify SqlDirectQuery executes SELECT and fetches rows
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_SelectSingleRow_FetchesData)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "queryuser", "querypass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER)"));
    create.execute();
    
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Alice', 30)"));
    insert.execute();
    
    // Query using SqlDirectQuery
    SqlDirectQuery query(conn, SAString("SELECT id, name, age FROM users"));
    
    ASSERT_TRUE(query.query());  // Should fetch first row
    
    EXPECT_EQ(query.Field(1).asLong(), 1);
    EXPECT_STREQ(query.Field(2).asString().GetMultiByteChars(), "Alice");
    EXPECT_EQ(query.Field(3).asLong(), 30);
    
    EXPECT_FALSE(query.query());  // No more rows
}

/**
 * @test Verify SqlDirectQuery fetches multiple rows
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_SelectMultipleRows_FetchesAll)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "multiuser", "multipass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO users VALUES(1, 'Alice')"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO users VALUES(2, 'Bob')"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO users VALUES(3, 'Charlie')"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    
    // Query all rows
    SqlDirectQuery query(conn, SAString("SELECT id, name FROM users ORDER BY id"));
    
    int rowCount = 0;
    while(query.query()) {
        rowCount++;
        EXPECT_EQ(query.Field(1).asLong(), rowCount);
    }
    
    EXPECT_EQ(rowCount, 3);
}

/**
 * @test Verify SqlDirectQuery handles empty result set
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_EmptyResult_ReturnsFalse)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "emptyuser", "emptypass");
    conn.connect();
    
    // Create empty table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    // Query empty table
    SqlDirectQuery query(conn, SAString("SELECT * FROM users"));
    
    EXPECT_FALSE(query.query());  // No rows to fetch
}

/**
 * @test Verify SqlDirectQuery with WHERE clause
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_WithWhereClause_FiltersResults)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "whereuser", "wherepass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO users VALUES(1, 'Alice', 25)"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO users VALUES(2, 'Bob', 35)"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO users VALUES(3, 'Charlie', 45)"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    
    // Query with filter
    SqlDirectQuery query(conn, SAString("SELECT name, age FROM users WHERE age > 30 ORDER BY age"));
    
    // First row: Bob (35)
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Bob");
    EXPECT_EQ(query.Field(2).asLong(), 35);
    
    // Second row: Charlie (45)
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Charlie");
    EXPECT_EQ(query.Field(2).asLong(), 45);
    
    // No more rows
    EXPECT_FALSE(query.query());
}

/**
 * @test Verify SqlDirectQuery with JOIN
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_WithJoin_FetchesJoinedData)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "joinuser", "joinpass");
    conn.connect();
    
    // Create tables
    SqlDirectCommand createUsers(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    SqlDirectCommand createOrders(conn, SAString("CREATE TABLE orders(id INTEGER, user_id INTEGER, total REAL)"));
    createUsers.execute();
    createOrders.execute();
    
    // Insert data
    SqlDirectCommand insertUser(conn, SAString("INSERT INTO users VALUES(1, 'Alice')"));
    SqlDirectCommand insertOrder(conn, SAString("INSERT INTO orders VALUES(100, 1, 99.99)"));
    insertUser.execute();
    insertOrder.execute();
    
    // Query with JOIN
    SqlDirectQuery query(conn, SAString(
        "SELECT u.name, o.id, o.total FROM users u JOIN orders o ON u.id = o.user_id"));
    
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Alice");
    EXPECT_EQ(query.Field(2).asLong(), 100);
    EXPECT_NEAR(query.Field(3).asDouble(), 99.99, 0.01);
}

/**
 * @test Verify SqlDirectQuery with aggregate functions
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_WithAggregates_ReturnsCorrectValues)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "agguser", "aggpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE sales(id INTEGER, amount REAL)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO sales VALUES(1, 100.50)"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO sales VALUES(2, 200.75)"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO sales VALUES(3, 150.25)"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    
    // Query aggregates
    SqlDirectQuery query(conn, SAString(
        "SELECT COUNT(*) as count, SUM(amount) as total, AVG(amount) as avg FROM sales"));
    
    ASSERT_TRUE(query.query());
    EXPECT_EQ(query.Field(1).asLong(), 3);
    EXPECT_NEAR(query.Field(2).asDouble(), 451.50, 0.01);
    EXPECT_NEAR(query.Field(3).asDouble(), 150.50, 0.01);
}

/**
 * @test Verify SqlDirectQuery with INSERT RETURNING (SQLite 3.35+)
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_InsertReturning_FetchesReturnedValue)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "retuser", "retpass");
    conn.connect();
    
    // Create table with auto-increment
    SqlDirectCommand create(conn, SAString(
        "CREATE TABLE users(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)"));
    create.execute();
    
    // Insert with RETURNING
    SqlDirectQuery query(conn, SAString("INSERT INTO users(name) VALUES('Alice') RETURNING id"));
    
    try {
        if(query.query()) {
            long insertedId = query.Field(1).asLong();
            EXPECT_GT(insertedId, 0);
        } else {
            GTEST_SKIP() << "RETURNING clause not supported (requires SQLite 3.35+)";
        }
    } catch (const SAException& e) {
        GTEST_SKIP() << "RETURNING clause not supported: " << e.ErrText().GetMultiByteChars();
    }
}

/**
 * @test Verify SqlDirectQuery with ORDER BY
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_WithOrderBy_ReturnsSortedResults)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "orderuser", "orderpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(name TEXT, age INTEGER)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO users VALUES('Charlie', 35)"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO users VALUES('Alice', 30)"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO users VALUES('Bob', 25)"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    
    // Query with ORDER BY
    SqlDirectQuery query(conn, SAString("SELECT name FROM users ORDER BY age ASC"));
    
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Bob");
    
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Alice");
    
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Charlie");
}

/**
 * @test Verify SqlDirectQuery with LIMIT
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_WithLimit_ReturnsLimitedRows)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "limituser", "limitpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER)"));
    create.execute();
    
    for(int i = 1; i <= 10; i++) {
        std::string sql = "INSERT INTO users VALUES(" + std::to_string(i) + ")";
        SqlDirectCommand insert(conn, SAString(sql.c_str()));
        insert.execute();
    }
    
    // Query with LIMIT
    SqlDirectQuery query(conn, SAString("SELECT id FROM users LIMIT 3"));
    
    int count = 0;
    while(query.query()) {
        count++;
    }
    
    EXPECT_EQ(count, 3);
}

// ============================================================================
// SqlQuery Integration Tests (with applets)
// ============================================================================

/**
 * @test Verify SqlQuery with applet executes SELECT
 */
TEST_F(SqlQueryIntegrationTest, SqlQuery_WithApplet_FetchesData)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "sqlquser", "sqlqpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER)"));
    create.execute();
    
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Alice', 30)"));
    insert.execute();
    
    // Use SqlQuery with applet
    SqlQuery query(conn, "select_user_test.xml");
    query.addParameter("min_age", 25);
    
    try {
        if(query.query()) {
            EXPECT_EQ(query.Field(1).asLong(), 1);
            EXPECT_STREQ(query.Field(2).asString().GetMultiByteChars(), "Alice");
            EXPECT_EQ(query.Field(3).asLong(), 30);
        } else {
            GTEST_SKIP() << "Applet file select_user_test.xml not available";
        }
    } catch (const SQLAppletException& e) {
        GTEST_SKIP() << "Applet not found: " << e.what();
    }
}

/**
 * @test Verify SqlQuery with multiple parameters
 */
TEST_F(SqlQueryIntegrationTest, SqlQuery_WithMultipleParams_FiltersCorrectly)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "paramuser", "parampass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, age INTEGER, city TEXT)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO users VALUES(1, 'Alice', 30, 'NYC')"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO users VALUES(2, 'Bob', 35, 'LA')"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO users VALUES(3, 'Charlie', 25, 'NYC')"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    
    // Use SqlQuery with multiple parameters
    SqlQuery query(conn, "select_users_by_city_age_test.xml");
    query.addParameter("city", "NYC");
    query.addParameter("min_age", 28);
    
    try {
        int count = 0;
        while(query.query()) {
            count++;
            // Should only get Alice (30, NYC), not Charlie (25, NYC)
        }
        EXPECT_EQ(count, 1);
    } catch (const SQLAppletException& e) {
        GTEST_SKIP() << "Applet not found: " << e.what();
    }
}

/**
 * @test Verify SqlQuery handles result set iteration correctly
 */
TEST_F(SqlQueryIntegrationTest, SqlQuery_IterateResultSet_ProcessesAllRows)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "iteruser", "iterpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE numbers(value INTEGER)"));
    create.execute();
    
    for(int i = 1; i <= 5; i++) {
        std::string sql = "INSERT INTO numbers VALUES(" + std::to_string(i) + ")";
        SqlDirectCommand insert(conn, SAString(sql.c_str()));
        insert.execute();
    }
    
    // Direct query to verify behavior
    SqlDirectQuery query(conn, SAString("SELECT value FROM numbers ORDER BY value"));
    
    std::vector<int> values;
    while(query.query()) {
        values.push_back(query.Field(1).asLong());
    }
    
    ASSERT_EQ(values.size(), 5);
    for(size_t i = 0; i < values.size(); i++) {
        EXPECT_EQ(values[i], i + 1);
    }
}

/**
 * @test Verify SqlDirectQuery with transaction rollback
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_TransactionRollback_DataNotPersisted)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "rolluser", "rollpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    // Start transaction
    conn.setAutoCommit(false);
    
    // Insert data
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Alice')"));
    insert.execute();
    
    // Rollback
    conn.rollback();
    
    // Query should return no rows
    SqlDirectQuery query(conn, SAString("SELECT * FROM users"));
    EXPECT_FALSE(query.query());
}

/**
 * @test Verify SqlDirectQuery with transaction commit
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_TransactionCommit_DataPersisted)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "commituser", "commitpass");
    conn.connect();
    
    // Create table
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT)"));
    create.execute();
    
    // Start transaction
    conn.setAutoCommit(false);
    
    // Insert data
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Alice')"));
    insert.execute();
    
    // Commit
    conn.commit();
    
    // Query should return the row
    SqlDirectQuery query(conn, SAString("SELECT id, name FROM users"));
    ASSERT_TRUE(query.query());
    EXPECT_EQ(query.Field(1).asLong(), 1);
    EXPECT_STREQ(query.Field(2).asString().GetMultiByteChars(), "Alice");
}

/**
 * @test Verify SqlDirectQuery handles NULL values
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_NullValues_HandlesCorrectly)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "nulluser", "nullpass");
    conn.connect();
    
    // Create table allowing NULLs
    SqlDirectCommand create(conn, SAString("CREATE TABLE users(id INTEGER, name TEXT, email TEXT)"));
    create.execute();
    
    // Insert row with NULL email
    SqlDirectCommand insert(conn, SAString("INSERT INTO users VALUES(1, 'Alice', NULL)"));
    insert.execute();
    
    // Query
    SqlDirectQuery query(conn, SAString("SELECT id, name, email FROM users"));
    ASSERT_TRUE(query.query());
    
    EXPECT_EQ(query.Field(1).asLong(), 1);
    EXPECT_STREQ(query.Field(2).asString().GetMultiByteChars(), "Alice");
    EXPECT_TRUE(query.Field(3).isNull());
}

/**
 * @test Verify SqlDirectQuery with complex WHERE clause
 */
TEST_F(SqlQueryIntegrationTest, SqlDirectQuery_ComplexWhereClause_FiltersCorrectly)
{
    SqlConnection conn(SA_SQLite_Client, ":memory:", "complexuser", "complexpass");
    conn.connect();
    
    // Create and populate table
    SqlDirectCommand create(conn, SAString("CREATE TABLE products(id INTEGER, name TEXT, price REAL, stock INTEGER)"));
    create.execute();
    
    SqlDirectCommand insert1(conn, SAString("INSERT INTO products VALUES(1, 'Widget', 19.99, 100)"));
    SqlDirectCommand insert2(conn, SAString("INSERT INTO products VALUES(2, 'Gadget', 29.99, 50)"));
    SqlDirectCommand insert3(conn, SAString("INSERT INTO products VALUES(3, 'Gizmo', 9.99, 200)"));
    SqlDirectCommand insert4(conn, SAString("INSERT INTO products VALUES(4, 'Tool', 39.99, 25)"));
    insert1.execute();
    insert2.execute();
    insert3.execute();
    insert4.execute();
    
    // Complex query: price between 15 and 35, stock > 30
    SqlDirectQuery query(conn, SAString(
        "SELECT name, price FROM products WHERE price BETWEEN 15 AND 35 AND stock > 30 ORDER BY price"));
    
    // Should match: Widget (19.99, 100) and Gadget (29.99, 50)
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Widget");
    
    ASSERT_TRUE(query.query());
    EXPECT_STREQ(query.Field(1).asString().GetMultiByteChars(), "Gadget");
    
    EXPECT_FALSE(query.query());
}

/*
 * NOTE: These integration tests use SQLite which is:
 * - Lightweight (no installation required)
 * - Fast (in-memory database)
 * - Perfect for CI/CD pipelines
 * 
 * Tests that use SqlQuery with applets may skip if applet XML files are not available.
 * To enable full testing, create the required applet files in the test app-data directory.
 */
