#include "sqlapplet.h"
#include "gtest/gtest.h"

#include <chrono>
#include <sstream>

using std::string;

namespace {
class SqlAppletTestFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    }
};
}

// ============================================================================
// Initialization Tests
// ============================================================================

TEST(AppletTests, InitTest)
{
    // Create object without pre initialization
    EXPECT_THROW({
        SQLApplet applet("applet");
    }, SQLAppletException);
}

// ============================================================================
// Path and Parsing Tests
// ============================================================================

TEST(AppletTests, WrongAppletPath)
{
    // Initialize project applet path
    SQLApplet::InitPathToApplets("XX:/No Path to Applets");
    SQLApplet applet("applet");
    try {
        applet.parse();
        FAIL() << "Expected SQLAppletException to be thrown";
    } catch(const SQLAppletException & e) {
        // Check that the error message contains the expected text
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Applet file not found") != std::string::npos);
    } catch (...) {
        FAIL() << "Expected SQLAppletException but got different exception type";
    }
}

TEST(AppletTests, InitAppletsAndParseConfig)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    EXPECT_NO_THROW(applet.parse());
}

// ============================================================================
// SQL Generation Tests
// ============================================================================

TEST_F(SqlAppletTestFixture, SqlTest)
{

    string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SQLApplet applet("test.xml");
    applet.addParameter("Money", 122.123);
    applet.addParameter("Height", 175);
    applet.addParameter("BirthTime", sysSecs, DataInfo::Time);
    applet.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    applet.addParameter("BirthDate", sysSecs, DataInfo::Date);
    applet.addParameter("Name", "Givi");

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";

    EXPECT_NO_THROW(applet.parse());
    // sql() now returns parameterized SQL with :name markers; use getDebugSql() for human-readable check
    EXPECT_TRUE(applet.getDebugSql().find(actual) != std::string::npos);
    // Verify parameterized SQL has markers, not inline values
    EXPECT_TRUE(applet.sql().find(":Money") != std::string::npos);
    EXPECT_TRUE(applet.sql().find(":Name") != std::string::npos);
}

TEST_F(SqlAppletTestFixture, SqlHybridDataTest)
{

    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SQLApplet applet("test.xml", {{"Money", "122.123000"}, {"Height", "175"}});
    applet.addParameter("BirthTime", sysSecs, DataInfo::Time);
    applet.addParameter("WholeDateTime", sysSecs, DataInfo::DateTime);
    applet.addParameter("BirthDate", sysSecs, DataInfo::Date);
    applet.addParameter("Name", "Givi");

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    EXPECT_NO_THROW(applet.parse());

    string expected = applet.getDebugSql();
    EXPECT_TRUE(applet.getDebugSql().find(actual) != string::npos);
}

TEST_F(SqlAppletTestFixture, SqlOnliInnerDataTest)
{

    string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    // Data without quotes
    SQLApplet applet("test.xml",{{"Money", "122.123000"},
                                {"Height", "175"},
                                {"BirthTime", "10:11:12"},
                                {"WholeDateTime", "2007-01-20 10:11:12"},
                                {"BirthDate", "2007-01-20"},
                                {"Name", "Givi"}
                                 });

    // Data with quotes (debug SQL shows escaped values for logging)
    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    EXPECT_NO_THROW(applet.parse());

    EXPECT_TRUE(applet.getDebugSql().find(actual) != string::npos);
}

// ============================================================================
// Default Values Test
// ============================================================================

TEST(AppletTests, SqlDefaultDataTest)
{
    // Use default values
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    SQLApplet applet("test.xml");

    EXPECT_NO_THROW(applet.parse());
    EXPECT_TRUE(applet.getDebugSql().find(actual) != string::npos);
}

// ============================================================================
// New State and API Tests
// ============================================================================

TEST(AppletTests, IsParsedState)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    
    EXPECT_FALSE(applet.isParsed());
    applet.parse();
    EXPECT_TRUE(applet.isParsed());
}

TEST(AppletTests, GetAppletPath)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    
    std::string path = applet.appletPath();
    EXPECT_TRUE(path.find("test.xml") != std::string::npos);
}

TEST(AppletTests, GetDescription)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    
    applet.parse();
    EXPECT_FALSE(applet.description().empty());
    EXPECT_TRUE(applet.description().find("Query selection users") != std::string::npos);
}

TEST(AppletTests, AddParameterTypes)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml");
    
    // Test all parameter types
    applet.addParameter("Height", 175);
    applet.addParameter("Money", 122.123);
    applet.addParameter("Name", "TestUser");
    applet.addParameter("Active", true);
    
    auto timePoint = timeFormatter::fromString("2007-01-20 10:11:12", DataInfo::DateTime);
    applet.addParameter("BirthTime", timePoint, DataInfo::Time);
    applet.addParameter("WholeDateTime", timePoint, DataInfo::DateTime);
    applet.addParameter("BirthDate", timePoint, DataInfo::Date);
    
    EXPECT_NO_THROW(applet.parse());
    EXPECT_FALSE(applet.sql().empty());
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST(AppletTests, MissingDescriptionTag)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("NoDescApplet.xml");
    
    try {
        applet.parse();
        FAIL() << "Expected exception for missing Description tag";
    } catch(const SQLAppletException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Description") != std::string::npos);
    }
}

TEST(AppletTests, MissingCodeTag)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("NoCodeApplet.xml");
    
    EXPECT_THROW({
        applet.parse();
    }, SQLAppletException);
}

TEST(AppletTests, MissingParameterName)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("NoNameParam.xml");
    
    try {
        applet.parse();
        FAIL() << "Expected exception for missing parameter Name";
    } catch(const SQLAppletException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Name") != std::string::npos);
    }
}

TEST(AppletTests, MissingParameterType)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("NoTypeParam.xml");
    
    try {
        applet.parse();
        FAIL() << "Expected exception for missing parameter Type";
    } catch(const SQLAppletException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("Type") != std::string::npos);
    }
}

TEST(AppletTests, MissingRequiredParameter)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, false); // Don't use defaults
    SQLApplet applet("test.xml");
    
    // Don't provide any parameters
    try {
        applet.parse();
        FAIL() << "Expected exception for missing required parameters";
    } catch(const SQLAppletException& e) {
        std::string errorMsg = e.what();
        EXPECT_TRUE(errorMsg.find("not provided") != std::string::npos ||
                    errorMsg.find("Required") != std::string::npos);
    }
}

// ============================================================================
// JSON Parameter Tests
// ============================================================================

TEST(AppletTests, ConstructorWithJsonParameters)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    
    std::string jsonParams = R"({
        "Money": "122.123000",
        "Height": "175",
        "BirthTime": "10:11:12",
        "WholeDateTime": "2007-01-20 10:11:12",
        "BirthDate": "2007-01-20",
        "Name": "JsonUser"
    })";
    
    SQLApplet applet("test.xml", jsonParams);
    EXPECT_NO_THROW(applet.parse());
    EXPECT_TRUE(applet.getDebugSql().find("'JsonUser'") != std::string::npos);
}

// ============================================================================
// NULL Value Tests
// ============================================================================

TEST(AppletTests, NullStringValue)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml", {
        {"Money", "100"},
        {"Height", "180"},
        {"BirthTime", "NULL"},
        {"WholeDateTime", "2007-01-20 10:11:12"},
        {"BirthDate", "2007-01-20"},
        {"Name", "NULL"}
    });
    
    EXPECT_NO_THROW(applet.parse());
    // In parameterized SQL, NULL value means the :Name marker is bound as null, not inlined
    EXPECT_TRUE(applet.getDebugSql().find("Name=NULL") != std::string::npos);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(AppletTests, EmptyParameterName)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    
    // This should work - empty string is valid
    EXPECT_NO_THROW(applet.addParameter("", "value"));
}

TEST(AppletTests, SpecialCharactersInValue)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml", {
        {"Money", "100"},
        {"Height", "180"},
        {"BirthTime", "10:11:12"},
        {"WholeDateTime", "2007-01-20 10:11:12"},
        {"BirthDate", "2007-01-20"},
        {"Name", "O'Brien"}  // Single quote in name
    });
    
    EXPECT_NO_THROW(applet.parse());
    // Debug SQL should show proper escaping for O'Brien
    EXPECT_TRUE(applet.getDebugSql().find("O''Brien") != std::string::npos);
    // Parameterized SQL should have :Name marker, not inline value
    EXPECT_TRUE(applet.sql().find(":Name") != std::string::npos);
}

TEST(AppletTests, MultipleParseCalls)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);
    SQLApplet applet("test.xml");
    
    EXPECT_NO_THROW(applet.parse());
    std::string firstSql = applet.sql();
    
    // Parse again - should work
    EXPECT_NO_THROW(applet.parse());
    std::string secondSql = applet.sql();
    
    EXPECT_EQ(firstSql, secondSql);
}

TEST(AppletTests, ParameterSubstitution)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml");
    
    applet.addParameter("Money", 999.99);
    applet.addParameter("Height", 200);
    applet.addParameter("BirthTime", "12:34:56", DataInfo::Time);
    applet.addParameter("WholeDateTime", "2000-01-01 00:00:00", DataInfo::DateTime);
    applet.addParameter("BirthDate", "2000-01-01", DataInfo::Date);
    applet.addParameter("Name", "CustomName");
    
    applet.parse();
    
    // Parameterized SQL uses :name markers (SQLAPI++ format), not inline values
    std::string sql = applet.sql();
    EXPECT_TRUE(sql.find(":Money") != std::string::npos);
    EXPECT_TRUE(sql.find(":Height") != std::string::npos);
    EXPECT_TRUE(sql.find(":BirthTime") != std::string::npos);
    EXPECT_TRUE(sql.find(":WholeDateTime") != std::string::npos);
    EXPECT_TRUE(sql.find(":BirthDate") != std::string::npos);
    EXPECT_TRUE(sql.find(":Name") != std::string::npos);
    
    // Ensure old :Name: placeholders are replaced
    EXPECT_FALSE(sql.find(":Money:") != std::string::npos);
    EXPECT_FALSE(sql.find(":Name:") != std::string::npos);
    
    // Debug SQL should contain the actual values for logging
    std::string debugSql = applet.getDebugSql();
    EXPECT_TRUE(debugSql.find("999.99") != std::string::npos);
    EXPECT_TRUE(debugSql.find("200") != std::string::npos);
    EXPECT_TRUE(debugSql.find("'12:34:56'") != std::string::npos);
    EXPECT_TRUE(debugSql.find("'CustomName'") != std::string::npos);
}

// ============================================================================
// SQL Injection Prevention Tests
// ============================================================================

TEST(AppletTests, IsValidIdentifier_ValidNames)
{
    EXPECT_TRUE(SQLApplet::isValidIdentifier("id"));
    EXPECT_TRUE(SQLApplet::isValidIdentifier("user_name"));
    EXPECT_TRUE(SQLApplet::isValidIdentifier("_private"));
    EXPECT_TRUE(SQLApplet::isValidIdentifier("COLUMN1"));
    EXPECT_TRUE(SQLApplet::isValidIdentifier("camelCase"));
    EXPECT_TRUE(SQLApplet::isValidIdentifier("a")); // single char
}

TEST(AppletTests, IsValidIdentifier_InvalidNames)
{
    // SQL injection attempts via identifiers
    EXPECT_FALSE(SQLApplet::isValidIdentifier("1;DROP TABLE users;--"));
    EXPECT_FALSE(SQLApplet::isValidIdentifier("name' OR '1'='1"));
    EXPECT_FALSE(SQLApplet::isValidIdentifier("name; DELETE FROM users"));
    EXPECT_FALSE(SQLApplet::isValidIdentifier("1name"));       // starts with digit
    EXPECT_FALSE(SQLApplet::isValidIdentifier(""));             // empty
    EXPECT_FALSE(SQLApplet::isValidIdentifier("col umn"));      // space
    EXPECT_FALSE(SQLApplet::isValidIdentifier("col-umn"));      // hyphen
    EXPECT_FALSE(SQLApplet::isValidIdentifier("col.umn"));      // dot
    EXPECT_FALSE(SQLApplet::isValidIdentifier("col\"umn"));     // double quote
    EXPECT_FALSE(SQLApplet::isValidIdentifier("col'umn"));      // single quote
    EXPECT_FALSE(SQLApplet::isValidIdentifier(std::string(64, 'a'))); // too long (>63)
}

TEST(AppletTests, SqlInjection_StringParameter_NotInlined)
{
    // Verify that string parameters are NOT inlined in parameterized SQL
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml", {{"Money", "100"}, {"Height", "180"},
                                  {"BirthTime", "10:11:12"}, {"WholeDateTime", "2007-01-20 10:11:12"},
                                  {"BirthDate", "2007-01-20"}});
    // SQL injection attempt via string parameter
    applet.addParameter("Name", "Givi' OR '1'='1");
    
    EXPECT_NO_THROW(applet.parse());
    
    // Parameterized SQL must NOT contain the injected value inline
    std::string paramSql = applet.sql();
    EXPECT_FALSE(paramSql.find("Givi' OR '1'='1") != std::string::npos)
        << "SQL injection string must NOT appear in parameterized SQL!";
    EXPECT_TRUE(paramSql.find(":Name") != std::string::npos)
        << "Parameter must be represented as a :Name marker, not inline value";
    
    // Debug SQL should still show the escaped value (safe for logging)
    std::string debugSql = applet.getDebugSql();
    EXPECT_TRUE(debugSql.find("Givi'' OR ''1''=''1") != std::string::npos)
        << "Debug SQL should show escaped version of the value";
}

TEST(AppletTests, SqlInjection_NumericParameter_NotStringInjection)
{
    // Verify that passing a string to a NUMERIC parameter won't allow injection
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml", {{"Height", "180"},
                                  {"BirthTime", "10:11:12"}, {"WholeDateTime", "2007-01-20 10:11:12"},
                                  {"BirthDate", "2007-01-20"}, {"Name", "Test"}});
    // Attempt SQL injection via numeric parameter
    applet.addParameter("Money", "1; DROP TABLE users;--");
    
    EXPECT_NO_THROW(applet.parse());
    
    // Parameterized SQL must use :name marker, not inline value
    std::string paramSql = applet.sql();
    EXPECT_TRUE(paramSql.find(":Money") != std::string::npos)
        << "Numeric parameter must use :name marker";
    EXPECT_FALSE(paramSql.find("DROP TABLE") != std::string::npos)
        << "SQL injection attempt must NOT appear in parameterized SQL!";
}

TEST(AppletTests, ParamBindings_PopulatedAfterParse)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml", {{"Money", "100"}, {"Height", "180"},
                                  {"BirthTime", "10:11:12"}, {"WholeDateTime", "2007-01-20 10:11:12"},
                                  {"BirthDate", "2007-01-20"}, {"Name", "TestUser"}});
    
    EXPECT_TRUE(applet.paramBindings().empty());
    applet.parse();
    
    // All 6 parameters should have bindings (none are FIELD type)
    EXPECT_EQ(applet.paramBindings().size(), 6);
    
    // Check first binding
    const auto& first = applet.paramBindings()[0];
    EXPECT_FALSE(first.name.empty());
    EXPECT_FALSE(first.value.empty());
}

