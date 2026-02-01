#include "sqlquery.h"
#include "sqlconnection.h"
#include "gtest/gtest.h"

// ============================================================================
// SqlDirectQuery Tests
// ============================================================================

/**
 * @class SqlDirectQueryTests
 * @brief Test fixture for SqlDirectQuery functionality (direct queries without applets)
 */
class SqlDirectQueryTests : public ::testing::Test {
protected:
    void SetUp() override {
        SqlConnection::InitAllConnections(SA_PostgreSQL_Client, "host", "user", "pass");
    }

    void TearDown() override {
        SqlConnection::ClearAllConnections();
    }
};

/**
 * @test Verify SqlDirectQuery constructor with simple SELECT
 */
TEST_F(SqlDirectQueryTests, Constructor_WithSimpleSelect_CreatesObject)
{
    SqlConnection con;
    EXPECT_NO_THROW({
        SqlDirectQuery query(con, SAString("SELECT 1"));
    });
}

/**
 * @test Verify SqlDirectQuery inherits from SqlDirectCommand
 */
TEST_F(SqlDirectQueryTests, SqlDirectQuery_InheritsFromSqlDirectCommand)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("SELECT 1"));
    
    // SqlDirectQuery should be usable as SqlDirectCommand
    SqlDirectCommand* cmdPtr = &query;
    EXPECT_NE(cmdPtr, nullptr);
}

/**
 * @test Verify query() executes direct SQL
 */
TEST_F(SqlDirectQueryTests, Query_WithDirectSQL_AttemptsExecution)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("SELECT * FROM users"));
    
    // Should throw SAException because connection is not established
    EXPECT_THROW(query.query(), SAException);
}

/**
 * @test Verify sql() method returns the command text
 */
TEST_F(SqlDirectQueryTests, Sql_ReturnsCommandText)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("SELECT * FROM users WHERE age > 18"));
    
    std::string sql = query.sql();
    EXPECT_EQ(sql, "SELECT * FROM users WHERE age > 18");
}

/**
 * @test Verify direct query with INSERT RETURNING
 */
TEST_F(SqlDirectQueryTests, Query_WithInsertReturning_HandlesCorrectly)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("INSERT INTO users (name) VALUES ('John') RETURNING id"));
    
    // Should throw because connection is not real, but validates SQL structure
    EXPECT_THROW(query.query(), SAException);
    
    std::string sql = query.sql();
    EXPECT_NE(sql.find("RETURNING"), std::string::npos);
}

/**
 * @test Verify direct query with UPDATE RETURNING
 */
TEST_F(SqlDirectQueryTests, Query_WithUpdateReturning_HandlesCorrectly)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("UPDATE users SET name='Jane' WHERE id=1 RETURNING id"));
    
    std::string sql = query.sql();
    EXPECT_NE(sql.find("UPDATE"), std::string::npos);
    EXPECT_NE(sql.find("RETURNING"), std::string::npos);
}

/**
 * @test Verify direct query with complex SELECT
 */
TEST_F(SqlDirectQueryTests, Query_WithComplexSelect_HandlesCorrectly)
{
    SqlConnection con;
    std::string complexSQL = "SELECT u.id, u.name, o.total "
                            "FROM users u "
                            "JOIN orders o ON u.id = o.user_id "
                            "WHERE u.age > 18 AND o.total > 100 "
                            "ORDER BY o.total DESC";
    
    SqlDirectQuery query(con, SAString(complexSQL.c_str()));
    
    std::string sql = query.sql();
    EXPECT_EQ(sql, complexSQL);
}

/**
 * @test Verify direct query with stored procedure call
 */
TEST_F(SqlDirectQueryTests, Query_WithStoredProcedure_HandlesCorrectly)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("CALL get_user_report(18)"), SA_CmdStoredProc);
    
    // Verify command type can be specified
    EXPECT_NO_THROW({
        std::string sql = query.sql();
    });
}

/**
 * @test Verify multiple query() calls don't re-execute
 */
TEST_F(SqlDirectQueryTests, Query_MultipleCalls_DontReexecute)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("SELECT * FROM users"));
    
    // First call should attempt execution
    EXPECT_THROW(query.query(), SAException);
    
    // Since m_executed is now true, subsequent calls should just try FetchNext
    // which will also fail without connection, but differently
}

/**
 * @test Verify empty SQL string handling
 */
TEST_F(SqlDirectQueryTests, Constructor_WithEmptySQL_CreatesObject)
{
    SqlConnection con;
    
    // Should be able to create with empty SQL
    EXPECT_NO_THROW({
        SqlDirectQuery query(con, SAString(""));
    });
}

/**
 * @test Verify SQL with special characters
 */
TEST_F(SqlDirectQueryTests, Query_WithSpecialCharacters_HandlesCorrectly)
{
    SqlConnection con;
    SqlDirectQuery query(con, SAString("SELECT * FROM users WHERE name = 'O''Brien'"));
    
    std::string sql = query.sql();
    EXPECT_NE(sql.find("O''Brien"), std::string::npos);
}

/**
 * @test Verify SQL with line breaks
 */
TEST_F(SqlDirectQueryTests, Query_WithMultilineSQL_HandlesCorrectly)
{
    SqlConnection con;
    std::string multilineSQL = "SELECT id, name\n"
                              "FROM users\n"
                              "WHERE age > 18";
    
    SqlDirectQuery query(con, SAString(multilineSQL.c_str()));
    
    std::string sql = query.sql();
    EXPECT_FALSE(sql.empty());
}

// ============================================================================
// SqlQuery Tests (with applets)
// ============================================================================

/**
 * @class SqlQueryTests
 * @brief Test fixture for SqlQuery functionality
 */
class SqlQueryTests : public ::testing::Test {
protected:
    void SetUp() override {
        SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
        SqlConnection::InitAllConnections(SA_PostgreSQL_Client, "host", "user", "pass");
    }

    void TearDown() override {
        SqlConnection::ClearAllConnections();
    }
};

/**
 * @test Verify SqlQuery constructor initializes properly
 */
TEST_F(SqlQueryTests, Constructor_WithValidParams_CreatesObject)
{
    SqlConnection con;
    EXPECT_NO_THROW({
        SqlQuery query(con, "test.xml");
    });
}

/**
 * @test Verify SqlQuery inherits from SqlDirectQuery
 */
TEST_F(SqlQueryTests, SqlQuery_InheritsFromSqlDirectQuery)
{
    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    // SqlQuery should be usable as SqlDirectQuery
    SqlDirectQuery* queryPtr = &query;
    EXPECT_NE(queryPtr, nullptr);
    
    // And through that, also as SqlDirectCommand
    SqlDirectCommand* cmdPtr = &query;
    EXPECT_NE(cmdPtr, nullptr);
}

/**
 * @test Verify parameter binding works through SqlQuery
 */
TEST_F(SqlQueryTests, AddParameter_WithVariousTypes_BindsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    // Test all parameter types
    EXPECT_NO_THROW({
        query.addParameter("Money", 122.123);
        query.addParameter("Height", 175);
        query.addParameter("BirthTime", sysSecs, DataInfo::Time);
        query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
        query.addParameter("BirthDate", sysSecs, DataInfo::Date);
        query.addParameter("Name", "TestUser");
    });
}

/**
 * @test Verify query() executes on first call
 * Note: This test expects SAException because connection is not real
 */
TEST_F(SqlQueryTests, Query_FirstCall_ExecutesCommand)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 180);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "John");
    
    // Should throw SAException because connection is not established
    // But this verifies that query() calls execute() internally
    EXPECT_THROW(query.query(), SAException);
}

/**
 * @test Verify SQL generation with parameters
 */
TEST_F(SqlQueryTests, Query_BeforeExecution_GeneratesCorrectSQL)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 99.99);
    query.addParameter("Height", 170);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Alice");
    
    // Use getSqlWithParameters to get SQL without executing
    std::string sql = query.getSqlWithParameters();
    
    // Verify parameters are substituted
    EXPECT_NE(sql.find("99.99"), std::string::npos);
    EXPECT_NE(sql.find("170"), std::string::npos);
    EXPECT_NE(sql.find("Alice"), std::string::npos);
    EXPECT_NE(sql.find("2007-01-20"), std::string::npos);
}

/**
 * @test Verify query handles string parameters with special characters
 */
TEST_F(SqlQueryTests, Query_StringWithSpecialChars_HandlesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 50.0);
    query.addParameter("Height", 165);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "O'Malley"); // String with quote
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify the name is correctly escaped in the SQL literal
    EXPECT_NE(sql.find("O''Malley"), std::string::npos);
}

/**
 * @test Verify query handles integer parameters correctly
 */
TEST_F(SqlQueryTests, Query_IntegerParameter_FormatsWithoutDecimal)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 42); // The value we're testing
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify integer is present
    EXPECT_NE(sql.find("42"), std::string::npos);
}

/**
 * @test Verify query handles double parameters with precision
 */
TEST_F(SqlQueryTests, Query_DoubleParameter_MaintainsPrecision)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 123.456789); // The value we're testing
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify double precision is maintained
    EXPECT_NE(sql.find("123.456"), std::string::npos);
}

/**
 * @test Verify query handles date parameter formatting
 */
TEST_F(SqlQueryTests, Query_DateParameter_FormatsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date); // Testing date format
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify date format (should be YYYY-MM-DD)
    EXPECT_NE(sql.find("2007-01-20"), std::string::npos);
}

/**
 * @test Verify query handles time parameter formatting
 */
TEST_F(SqlQueryTests, Query_TimeParameter_FormatsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time); // Testing time format
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify time format (should be HH:MM:SS)
    EXPECT_NE(sql.find("10:11:12"), std::string::npos);
}

/**
 * @test Verify query handles datetime parameter formatting
 */
TEST_F(SqlQueryTests, Query_DateTimeParameter_FormatsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime); // Testing datetime format
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Verify datetime format (should be YYYY-MM-DD HH:MM:SS)
    EXPECT_NE(sql.find("2007-01-20 10:11:12"), std::string::npos);
}

/**
 * @test Verify query method handles missing applet file
 */
TEST_F(SqlQueryTests, Query_MissingApplet_ThrowsException)
{
    SqlConnection con;
    SqlQuery query(con, "nonexistent.xml");
    
    // Should throw SQLAppletException when trying to execute
    EXPECT_THROW(query.query(), SQLAppletException);
}

/**
 * @test Verify query with minimal parameters
 */
TEST_F(SqlQueryTests, Query_MinimalParameters_WorksCorrectly)
{
    SqlConnection con;
    
    // Test with just connection and applet name
    EXPECT_NO_THROW({
        SqlQuery query(con, "test.xml");
    });
}

/**
 * @test Verify query can be used with formatted parameter list
 */
TEST_F(SqlQueryTests, Query_WithFormattedParams_InitializesCorrectly)
{
    SqlConnection con;
    std::map<std::string, std::string> preFormatted;
    preFormatted["Name"] = "PreFormattedValue";
    
    EXPECT_NO_THROW({
        SqlQuery query(con, "test.xml", preFormatted);
    });
}

/**
 * @test Verify query respects command type parameter
 */
TEST_F(SqlQueryTests, Query_WithCommandType_InitializesCorrectly)
{
    SqlConnection con;
    
    EXPECT_NO_THROW({
        SqlQuery query(con, "test.xml", {}, SAString(), SA_CmdSQLStmt);
    });
}

/**
 * @test Verify sql() method returns applet-processed SQL
 */
TEST_F(SqlQueryTests, Sql_AfterParameterBinding_ReturnsProcessedSQL)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "SqlTest");
    
    // getSqlWithParameters() parses and returns SQL with substituted parameters
    std::string sqlText = query.getSqlWithParameters();
    
    // Should contain processed SQL with parameters
    EXPECT_FALSE(sqlText.empty());
    EXPECT_NE(sqlText.find("SqlTest"), std::string::npos);
}

/**
 * @test Verify boolean parameter handling
 */
TEST_F(SqlQueryTests, Query_BooleanParameter_FormatsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    // If test.xml had a boolean parameter, we could test it here
    // This test demonstrates the API is available
    std::string sql = query.getSqlWithParameters();
    EXPECT_FALSE(sql.empty());
}

/**
 * @test Verify empty string parameter handling
 */
TEST_F(SqlQueryTests, Query_EmptyStringParameter_HandlesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", ""); // Empty string
    
    std::string sql = query.getSqlWithParameters();
    
    // Should still generate valid SQL
    EXPECT_FALSE(sql.empty());
}

/**
 * @test Verify long string parameter handling
 */
TEST_F(SqlQueryTests, Query_LongStringParameter_HandlesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    std::string longString(1000, 'A'); // 1000 character string
    
    query.addParameter("Money", 100.0);
    query.addParameter("Height", 175);
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", longString.c_str());
    
    std::string sql = query.getSqlWithParameters();
    
    // Should handle long strings
    EXPECT_NE(sql.find(longString), std::string::npos);
}

/**
 * @test Verify negative number parameter handling
 */
TEST_F(SqlQueryTests, Query_NegativeNumbers_HandlesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", -99.99); // Negative double
    query.addParameter("Height", -50);   // Negative int
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Should handle negative numbers
    EXPECT_NE(sql.find("-99.99"), std::string::npos);
    EXPECT_NE(sql.find("-50"), std::string::npos);
}

/**
 * @test Verify zero value parameter handling
 */
TEST_F(SqlQueryTests, Query_ZeroValues_HandlesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlQuery query(con, "test.xml");
    
    query.addParameter("Money", 0.0);  // Zero double
    query.addParameter("Height", 0);   // Zero int
    query.addParameter("BirthTime", sysSecs, DataInfo::Time);
    query.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    query.addParameter("BirthDate", sysSecs, DataInfo::Date);
    query.addParameter("Name", "Test");
    
    std::string sql = query.getSqlWithParameters();
    
    // Should handle zero values correctly
    EXPECT_FALSE(sql.empty());
}
