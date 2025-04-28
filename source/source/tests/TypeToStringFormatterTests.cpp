#include "../TypeToStringFormatter.h"
#include "gtest/gtest.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <chrono>


TEST(FormaterTest, TimeFormatHelperTests)
{
    std::chrono::sys_seconds sysSecs;
    std::string input = "0-0-0 10:11:12";
    EXPECT_THROW(TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime), std::invalid_argument);

    input = "1211-10-11 10:11:12";
    EXPECT_NO_THROW(sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime));

    try {
        TimeFormatHelper::chronoSysSecToString(sysSecs, DataInfo::Double);
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(FORMATER_ERR_WRONG_DATE_TYME_TYPE, err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(sysSecs, DataInfo::DateTime), "1211-10-11 10:11:12");
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(sysSecs, DataInfo::DateTimeNoSec), "1211-10-11 10:11");
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(sysSecs, DataInfo::Date), "1211-10-11");
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(sysSecs, DataInfo::Time), "10:11:12");
}


TEST(TypeToStringFormatterTests, TypeToStringFormatterWrongParamValueTests)
{
    FormatterDataType dataType;
    dataType = 10;

    TypeToStringFormatter formatter;
    formatter.AddDataInfo("IntType", dataType);
    try {
        formatter.value("WrongParamName");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME, err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    try {
        formatter.dataInfo("WrongParamName");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME, err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    DataInfo info = formatter.dataInfo("IntType");
    EXPECT_EQ(info.value, "10");
    EXPECT_EQ(info.type, DataInfo::Int);
    EXPECT_EQ(info.param, "IntType");
}

TEST(TypeToStringFormatterTests, AddAllTypesExceptDateTime)
{
    std::vector paramNameList = {"Int", "Double", "Strig", "Bool"};

    TypeToStringFormatter formatter;
    FormatterDataType dataType;

    dataType = 10;
    formatter.AddDataInfo(paramNameList[0], dataType);

    dataType = 11.11;
    formatter.AddDataInfo(paramNameList[1], dataType);

    dataType = "RandomString";
    formatter.AddDataInfo(paramNameList[2], dataType);

    dataType = true;
    formatter.AddDataInfo(paramNameList[3], dataType);

    std::map<string, string> expected = {{"Int", "10"},
                                         {"Double", "11.110000"},// six digits by default
                                         {"Strig", "RandomString"},
                                         {"Bool", "1"}};

    std::map<string, string> actual = formatter.formattedParamValueList();

    // Assert if wrong
    ASSERT_EQ(expected, actual);
}

TEST(TypeToStringFormatterTests, TypeToStringFormatterTests)
{
    TypeToStringFormatter formatter;
    std::chrono::sys_seconds currentTime = TimeFormatHelper::stringTochronoSysSec("2007-01-20 11:22:33", DataInfo::DateTime);

    std::vector paramNameList = {"DateTime", "DateTimeNoSec", "Date", "Time"};
    formatter.AddDataInfo(paramNameList[0], currentTime, DataInfo::DateTime);
    formatter.AddDataInfo(paramNameList[1], currentTime, DataInfo::DateTimeNoSec);
    formatter.AddDataInfo(paramNameList[2], currentTime, DataInfo::Date);
    formatter.AddDataInfo(paramNameList[3], currentTime, DataInfo::Time);

    std::map<string, string> expected = {{"DateTime", "2007-01-20 11:22:33"},
                                         {"DateTimeNoSec", "2007-01-20 11:22"},
                                         {"Date", "2007-01-20"},
                                         {"Time", "11:22:33"}};
    std::map<string, string> actual = formatter.formattedParamValueList();

    // Assert if wrong
    ASSERT_EQ(expected, actual);
    for(const auto & [param, value] : actual) {
        EXPECT_TRUE(std::find(std::begin(paramNameList), std::end(paramNameList), param) != std::end(paramNameList));
    }
    // Test TypeToStringFormatter::value
    for(const auto & param : paramNameList) {
        EXPECT_EQ(actual[param], formatter.value(param));
    }
}

TEST(TypeToStringFormatterTests, TypeToStringFormatterStringsTests)
{
    const char * currentTimeStr("2007-01-20 11:22:33");

    TypeToStringFormatter formatter;
    formatter.AddDataInfo("DateTime", currentTimeStr, DataInfo::DateTime);
    formatter.AddDataInfo("DateTimeNoSec", currentTimeStr, DataInfo::DateTimeNoSec);
    formatter.AddDataInfo("Date", currentTimeStr, DataInfo::Date);
    formatter.AddDataInfo("Time", currentTimeStr, DataInfo::Time);

    std::map<string, string> expected = {{"DateTime", "2007-01-20 11:22:33"},
                                         {"DateTimeNoSec", "2007-01-20 11:22"},
                                         {"Date", "2007-01-20"},
                                         {"Time", "11:22:33"}};
    std::map<string, string> actual = formatter.formattedParamValueList();

    // Assert if wrong
    ASSERT_EQ(expected, actual);
}

TEST(TypeToStringFormatterTests, TypeToStringFormatterToTimeTests)
{

    TypeToStringFormatter formatter;
    FormatterDataType dataType;

    dataType = 10;
    formatter.AddDataInfo("IntType", dataType);

    try {
        formatter.toTime("WrongParamName");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(FORMATER_ERR_WRONG_KEY_PARAMETER_NAME, err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    try {
        formatter.toTime("IntType");
    } catch(const std::invalid_argument & err) {
        // and this tests that it has the correct message
        EXPECT_STREQ(FORMATER_ERR_WRONG_DATE_TYME_TYPE, err.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    const char * currentTimeStr("1-1-1 11:22:00");

    formatter.AddDataInfo("DateTime", currentTimeStr, DataInfo::DateTime);
    formatter.AddDataInfo("DateTimeNoSec", currentTimeStr, DataInfo::DateTimeNoSec);
    formatter.AddDataInfo("Date", currentTimeStr, DataInfo::Date);
    formatter.AddDataInfo("Time", currentTimeStr, DataInfo::Time);

    EXPECT_NO_THROW(formatter.toTime("DateTime"));
    EXPECT_NO_THROW(formatter.toTime("DateTimeNoSec"));
    EXPECT_NO_THROW(formatter.toTime("Date"));
    EXPECT_NO_THROW(formatter.toTime("Time"));
}



