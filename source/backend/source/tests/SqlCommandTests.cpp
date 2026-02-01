#include "sqlcommand.h"  // Now includes both SqlDirectCommand and SqlCommand
#include "sqlconnection.h"
#include "gtest/gtest.h"

// PostgreSQL RETURNING clause examples:
// https://stackoverflow.com/questions/2944297/postgresql-function-for-last-inserted-id
// INSERT INTO persons (lastname,firstname) VALUES ('Smith', 'John') RETURNING id;
// UPDATE customer SET "NICNAME" = 'Givi' WHERE "UID" = '18f23eaf-286b-4939-875c-8d2c5d8ec8d9' RETURNING "UID";
// 
// SQLAPI result set check:
// https://www.sqlapi.com/HowTo/fetch/
// bool is_result = cmd.isResultSet();

/**
 * @class SqlCommandTests
 * @brief Test fixture for SqlCommand functionality
 */
class SqlCommandTests : public ::testing::Test {
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
 * @test Verify parameter binding and SQL generation
 * Tests all parameter types: double, int, time, datetime, date, string
 */
TEST_F(SqlCommandTests, AddParameter_AllTypes_GeneratesCorrectSQL)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SqlConnection con;
    SqlCommand command(con, "test.xml");
    
    // Add various parameter types
    command.addParameter("Money", 122.123);
    command.addParameter("Height", 175);
    command.addParameter("BirthTime", sysSecs, DataInfo::Time);
    command.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    command.addParameter("BirthDate", sysSecs, DataInfo::Date);
    command.addParameter("Name", "Givi");
    
    // Should throw because connection is not established
    EXPECT_THROW(command.execute(), SAException);

    // Verify parameter substitution
    std::string expected = "Money=122.123000,Height=175,BirthTime='10:11:12',"
                          "WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    std::string actual = command.CommandText().GetMultiByteChars();

    EXPECT_NE(actual.find(expected), std::string::npos);
}

/**
 * @test Verify string parameter escaping
 */
TEST_F(SqlCommandTests, AddParameter_StringWithQuotes_EscapesCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);
    
    SqlConnection con;
    SqlCommand command(con, "test.xml");
    
    // Add all required parameters from test.xml
    command.addParameter("Money", 100.0);
    command.addParameter("Height", 175);
    command.addParameter("BirthTime", sysSecs, DataInfo::Time);
    command.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    command.addParameter("BirthDate", sysSecs, DataInfo::Date);
    command.addParameter("Name", "O'Brien"); // The parameter we're testing
    
    std::string sql = command.getSqlWithParameters();
    
    // SQLApplet escapes single quotes for safe SQL literals (Postgres compatible)
    EXPECT_NE(sql.find("O''Brien"), std::string::npos) << "Parameter should be substituted and escaped";
}

/**
 * @test Verify integer parameter
 */
TEST_F(SqlCommandTests, AddParameter_Integer_FormatsCorrectly)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);
    
    SqlConnection con;
    SqlCommand command(con, "test.xml");
    
    // Add all required parameters from test.xml
    command.addParameter("Money", 100.0);
    command.addParameter("Height", 42); // The parameter we're testing
    command.addParameter("BirthTime", sysSecs, DataInfo::Time);
    command.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    command.addParameter("BirthDate", sysSecs, DataInfo::Date);
    command.addParameter("Name", "Test");
    
    std::string sql = command.getSqlWithParameters();
    EXPECT_NE(sql.find("42"), std::string::npos);
}

/**
 * @test Verify double parameter precision
 */
TEST_F(SqlCommandTests, AddParameter_Double_MaintainsPrecision)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);
    
    SqlConnection con;
    SqlCommand command(con, "test.xml");
    
    // Add all required parameters from test.xml
    command.addParameter("Money", 123.456789); // The parameter we're testing
    command.addParameter("Height", 175);
    command.addParameter("BirthTime", sysSecs, DataInfo::Time);
    command.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    command.addParameter("BirthDate", sysSecs, DataInfo::Date);
    command.addParameter("Name", "Test");
    
    std::string sql = command.getSqlWithParameters();
    EXPECT_NE(sql.find("123.456"), std::string::npos);
}

/**
 * @test Verify SqlDirectCommand works without applet
 */
TEST_F(SqlCommandTests, SqlDirectCommand_ExecutesDirect)
{
    SqlConnection con;
    SqlDirectCommand command(con, SAString("SELECT 1"));
    
    // Should throw because connection is not established
    EXPECT_THROW(command.execute(), SAException); // Use lowercase execute()
    
    // Verify SQL text
    std::string sql = command.sql();
    EXPECT_EQ(sql, "SELECT 1");
}
