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
    EXPECT_TRUE(applet.sql().find(actual) != std::string::npos);
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

    string expected = applet.sql();
    EXPECT_TRUE(applet.sql().find(actual) != string::npos);
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

    // Data with quotes
    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    EXPECT_NO_THROW(applet.parse());

    string expected = applet.sql();
    EXPECT_TRUE(applet.sql().find(actual) != string::npos);
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
    EXPECT_TRUE(applet.sql().find(actual) != string::npos);
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
    EXPECT_TRUE(applet.sql().find("JsonUser") != std::string::npos);
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
    std::string sql = applet.sql();
    EXPECT_TRUE(sql.find("Name=NULL") != std::string::npos);
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
    EXPECT_TRUE(applet.sql().find("O''Brien") != std::string::npos);
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
    
    std::string sql = applet.sql();
    EXPECT_TRUE(sql.find("999.99") != std::string::npos);
    EXPECT_TRUE(sql.find("200") != std::string::npos);
    EXPECT_TRUE(sql.find("12:34:56") != std::string::npos);
    EXPECT_TRUE(sql.find("2000-01-01 00:00:00") != std::string::npos);
    EXPECT_TRUE(sql.find("2000-01-01") != std::string::npos);
    EXPECT_TRUE(sql.find("CustomName") != std::string::npos);
    
    // Ensure placeholders are replaced
    EXPECT_FALSE(sql.find(":Money:") != std::string::npos);
    EXPECT_FALSE(sql.find(":Name:") != std::string::npos);
}

