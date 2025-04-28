#include "sqlapplet.h"
#include "gtest/gtest.h"

#include <chrono>
#include <sstream>

namespace {
string sqlExpectedUnformated = "Money=:Money,Height=:Height,BirthTime=:BirthTime,BirthDateTime=:BirthDateTime,BirthDate=:BirthDate,Name=:Name";
}

TEST(AppletTests, InitTest)
{
    // Create object without pre initialization
    try {
        SQLApplet applet("applet");
    } catch(const SQLAppletException & e) {
        // and this tests that it has the correct message
        EXPECT_STREQ(APPLET_ERR_INIT, e.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}

TEST(AppletTests, WrongAppletPath)
{
    // Initialize project applet path
    SQLApplet::InitPathToApplets("XX:/No Path to Applets");
    SQLApplet applet("applet");
    try {
        applet.parse();
    } catch(const SQLAppletException & e) {
        // and this tests that it has the correct message
        EXPECT_STREQ(APPLET_ERR_WRONG_PATH, e.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}

TEST(AppletTests, InitAppletsAndParseConfig)
{
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SQLApplet applet("test.xml");
    EXPECT_NO_THROW(applet.parse());
}

TEST(AppletTests, SqlTest)
{

    std::string input = "2007-01-20 10:11:12";
    std::chrono::sys_seconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SQLApplet applet("test.xml");
    applet.AddDataInfo("Money", 122.123);
    applet.AddDataInfo("Height", 175);
    applet.AddDataInfo("BirthTime", sysSecs, DataInfo::Time);
    applet.AddDataInfo("WholeDateTime", sysSecs, DataInfo::DateTime);
    applet.AddDataInfo("BirthDate", sysSecs, DataInfo::Date);
    applet.AddDataInfo("Name", "Givi");

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";

    EXPECT_NO_THROW(applet.parse());
    EXPECT_TRUE(applet.sql().find(actual) != std::string::npos);
}

TEST(AppletTests, SqlHybridDataTest)
{

    std::string input = "2007-01-20 10:11:12";
    std::chrono::sys_seconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SQLApplet applet("test.xml", {{"Money", "122.123000"}, {"Height", "175"}});
    applet.AddDataInfo("BirthTime", sysSecs, DataInfo::Time);
    applet.AddDataInfo("WholeDateTime", sysSecs, DataInfo::DateTime);
    applet.AddDataInfo("BirthDate", sysSecs, DataInfo::Date);
    applet.AddDataInfo("Name", "Givi");

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    EXPECT_NO_THROW(applet.parse());

    string expected = applet.sql();
    EXPECT_TRUE(applet.sql().find(actual) != std::string::npos);
}

TEST(AppletTests, SqlOnliInnerDataTest)
{

    std::string input = "2007-01-20 10:11:12";
    std::chrono::sys_seconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

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
    EXPECT_TRUE(applet.sql().find(actual) != std::string::npos);
}

TEST(AppletTests, SqlDefaultDataTest)
{
    // Use default values
    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH, true);

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    SQLApplet applet("test.xml");

    EXPECT_NO_THROW(applet.parse());
    EXPECT_TRUE(applet.sql().find(actual) != std::string::npos);
}

