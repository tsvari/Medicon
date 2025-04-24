#include "TypeToStringFormatter.h"
#include "gtest/gtest.h"

#include <ctime>
#include <iomanip>
#include <sstream>


namespace {
time_t getDateTime() {
    struct tm tm{};
    std::istringstream dateTimeValue("2007-01-20 11:22:33");

    dateTimeValue >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::mktime(&tm);
}
}
// Test suite for the Calculator class
TEST(FormaterTest, AddAllDateTime)
{
    TypeToStringFormatter formatter;
    time_t currentTime = getDateTime();

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

    for(const auto & [param, value] : actual) {
        EXPECT_TRUE(std::find(std::begin(paramNameList), std::end(paramNameList), param) != std::end(paramNameList));
    }
    // Assert if wrong
    ASSERT_EQ(expected, actual);

    // Test TypeToStringFormatter::value
    for(const auto & param : paramNameList) {
        EXPECT_EQ(actual[param], formatter.value(param));
    }
    // Test local date and time
    for(const auto & param : paramNameList) {
        time_t value = formatter.toTime(param);
        EXPECT_TRUE(value > -1);

        DataInfo info = formatter.dataInfo(param);

        using namespace TimeFormatHelper;
        EXPECT_EQ(timeToString(value, info.type), info.value);
    }
}

TEST(FormaterTest, AddAllTypesExceptDateTime)
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

