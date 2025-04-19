#include "TypeToStringFormatter.h"
#include "gtest/gtest.h"

// Test suite for the Calculator class
TEST(FormaterTest, AddAllDateTime) {

    TypeToStringFormatter formatter;
    time_t currentTime;
    time(&currentTime);

    std::vector paramNameList = {"DateTime", "DateTimeNoSec", "Date", "Time"};

    formatter.AddDataInfo(paramNameList[0], currentTime, DataInfo::DateTime);
    formatter.AddDataInfo(paramNameList[1], currentTime, DataInfo::DateTimeNoSec);
    formatter.AddDataInfo(paramNameList[2], currentTime, DataInfo::Date);
    formatter.AddDataInfo(paramNameList[3], currentTime, DataInfo::Time);

    std::map paramValueList = formatter.formattedParamValueList();

    for(const auto & [param, value] : paramValueList) {
        EXPECT_TRUE(std::find(std::begin(paramNameList), std::end(paramNameList), param) != std::end(paramNameList));
    }
    for(const auto & param : paramNameList) {
        EXPECT_TRUE(paramValueList.contains(param));
    }

}

