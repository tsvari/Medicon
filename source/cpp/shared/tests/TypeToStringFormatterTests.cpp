#include "../TypeToStringFormatter.h"
#include "gtest/gtest.h"
#include <chrono>
#include <format>



// ============================================================================
// Time Formatting Helper Tests
// ============================================================================

class TimeFormatterTest : public ::testing::Test {
protected:
    const std::string validDateTime = "1211-10-11 10:11:12";
    const std::string validDate = "2007-01-20";
    const std::string validTime = "10:11:12";
    std::chrono::milliseconds sampleTime;

    void SetUp() override {
        sampleTime = timeFormatter::fromString(validDateTime, DataInfo::DateTime);
    }
};

TEST_F(TimeFormatterTest, ParseInvalidDateTime) {
    std::string invalid = "0-0-0 10:11:12";
    EXPECT_THROW(timeFormatter::fromString(invalid, DataInfo::DateTime), FormatterException);
}

TEST_F(TimeFormatterTest, ParseValidDateTime) {
    EXPECT_NO_THROW(timeFormatter::fromString(validDateTime, DataInfo::DateTime));
}

TEST_F(TimeFormatterTest, ThrowOnWrongTypeForDateTime) {
    EXPECT_THROW(
        timeFormatter::toString(sampleTime, DataInfo::Double),
        FormatterException
        );
}

TEST_F(TimeFormatterTest, FormatDateTime) {
    EXPECT_EQ(timeFormatter::toString(sampleTime, DataInfo::DateTime), "1211-10-11 10:11:12");
}

TEST_F(TimeFormatterTest, FormatDateTimeNoSec) {
    EXPECT_EQ(timeFormatter::toString(sampleTime, DataInfo::DateTimeNoSec), "1211-10-11 10:11");
}

TEST_F(TimeFormatterTest, FormatDate) {
    EXPECT_EQ(timeFormatter::toString(sampleTime, DataInfo::Date), "1211-10-11");
}

TEST_F(TimeFormatterTest, FormatTime) {
    EXPECT_EQ(timeFormatter::toString(sampleTime, DataInfo::Time), "10:11:12");
}

TEST_F(TimeFormatterTest, ConvertInt64ToDateTime) {
    int64_t milliseconds = sampleTime.count();
    std::string result = timeFormatter::toString(milliseconds, DataInfo::DateTime);
    EXPECT_EQ(result, "1211-10-11 10:11:12");
}

TEST_F(TimeFormatterTest, ParseTimeWithoutDate) {
    // Time type should be parsed correctly even without date
    EXPECT_NO_THROW(timeFormatter::fromString("10:11:12", DataInfo::Time));
}

TEST_F(TimeFormatterTest, GetCurrentTime) {
    auto now = timeFormatter::now();
    EXPECT_GT(now.count(), 0);
}

TEST_F(TimeFormatterTest, IsDateTimeType) {
    EXPECT_TRUE(timeFormatter::isDateTimeType(DataInfo::DateTime));
    EXPECT_TRUE(timeFormatter::isDateTimeType(DataInfo::DateTimeNoSec));
    EXPECT_TRUE(timeFormatter::isDateTimeType(DataInfo::Date));
    EXPECT_TRUE(timeFormatter::isDateTimeType(DataInfo::Time));

    EXPECT_FALSE(timeFormatter::isDateTimeType(DataInfo::Int));
    EXPECT_FALSE(timeFormatter::isDateTimeType(DataInfo::String));
    EXPECT_FALSE(timeFormatter::isDateTimeType(DataInfo::Double));
}

TEST_F(TimeFormatterTest, GenerateUniqueId) {
    auto id1 = timeFormatter::generateUniqueId();
    auto id2 = timeFormatter::generateUniqueId();

    EXPECT_EQ(id1.size(), 64);  // 32 bytes * 2 hex chars
    EXPECT_EQ(id2.size(), 64);
    EXPECT_NE(id1, id2);  // Should be different
}

// ============================================================================
// TypeToStringFormatter Basic Tests
// ============================================================================

class FormatterTest : public ::testing::Test {
protected:
    TypeToStringFormatter formatter;
};

TEST_F(FormatterTest, EmptyFormatterState) {
    EXPECT_TRUE(formatter.empty());
    EXPECT_EQ(formatter.size(), 0);
}

TEST_F(FormatterTest, AddIntParameter) {
    formatter.addParameter("Height", FormatterValue{175});

    EXPECT_FALSE(formatter.empty());
    EXPECT_EQ(formatter.size(), 1);
    EXPECT_TRUE(formatter.contains("Height"));

    auto value = formatter.getValue("Height");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "175");
}

TEST_F(FormatterTest, AddInt64Parameter) {
    formatter.addParameter("BigNum", FormatterValue{9223372036854775807LL});

    auto value = formatter.getValueOrThrow("BigNum");
    EXPECT_EQ(value, "9223372036854775807");
}

TEST_F(FormatterTest, AddDoubleParameter) {
    formatter.addParameter("Money", FormatterValue{122.123});

    auto value = formatter.getValueOrThrow("Money");
    // std::to_string for double includes 6 decimal places
    std::string expected = std::to_string(122.123);
    EXPECT_EQ(value, expected);
}

TEST_F(FormatterTest, AddStringParameter) {
    formatter.addParameter("Name", FormatterValue{std::string("John")});

    auto value = formatter.getValueOrThrow("Name");
    EXPECT_EQ(value, "John");
}

TEST_F(FormatterTest, AddBoolParameter) {
    formatter.addParameter("Active", FormatterValue{true});
    formatter.addParameter("Deleted", FormatterValue{false});

    EXPECT_EQ(formatter.getValueOrThrow("Active"), "1");
    EXPECT_EQ(formatter.getValueOrThrow("Deleted"), "0");
}

TEST_F(FormatterTest, AddMultipleParameters) {
    formatter.addParameter("Int", FormatterValue{10});
    formatter.addParameter("Double", FormatterValue{11.11});
    formatter.addParameter("String", FormatterValue{std::string("RandomString")});
    formatter.addParameter("Bool", FormatterValue{true});

    EXPECT_EQ(formatter.size(), 4);

    auto map = formatter.toMap();
    EXPECT_EQ(map.size(), 4);
    EXPECT_EQ(map["Int"], "10");
    EXPECT_EQ(map["String"], "RandomString");
    EXPECT_EQ(map["Bool"], "1");
}

TEST_F(FormatterTest, GetNonExistentParameter) {
    auto value = formatter.getValue("NotExists");
    EXPECT_FALSE(value.has_value());
}

TEST_F(FormatterTest, GetOrThrowNonExistentParameter) {
    EXPECT_THROW(
        formatter.getValueOrThrow("NotExists"),
        FormatterException
        );
}

TEST_F(FormatterTest, ExceptionContainsParameterName) {
    try {
        formatter.getValueOrThrow("MissingParam");
        FAIL() << "Expected FormatterException";
    } catch (const FormatterException& e) {
        std::string message = e.what();
        EXPECT_TRUE(message.find("MissingParam") != std::string::npos);
    }
}

TEST_F(FormatterTest, ContainsCheck) {
    formatter.addParameter("Exists", FormatterValue{42});

    EXPECT_TRUE(formatter.contains("Exists"));
    EXPECT_FALSE(formatter.contains("NotExists"));
}

TEST_F(FormatterTest, ClearParameters) {
    formatter.addParameter("Param1", FormatterValue{1});
    formatter.addParameter("Param2", FormatterValue{2});

    EXPECT_EQ(formatter.size(), 2);

    formatter.clear();

    EXPECT_TRUE(formatter.empty());
    EXPECT_EQ(formatter.size(), 0);
    EXPECT_FALSE(formatter.contains("Param1"));
}

// ============================================================================
// DateTime Parameter Tests
// ============================================================================

TEST_F(FormatterTest, AddDateTimeFromChrono) {
    auto time = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);

    formatter.addParameter("DateTime", time, DataInfo::DateTime);
    formatter.addParameter("DateTimeNoSec", time, DataInfo::DateTimeNoSec);
    formatter.addParameter("Date", time, DataInfo::Date);
    formatter.addParameter("Time", time, DataInfo::Time);

    EXPECT_EQ(formatter.getValueOrThrow("DateTime"), "2007-01-20 11:22:33");
    EXPECT_EQ(formatter.getValueOrThrow("DateTimeNoSec"), "2007-01-20 11:22");
    EXPECT_EQ(formatter.getValueOrThrow("Date"), "2007-01-20");
    EXPECT_EQ(formatter.getValueOrThrow("Time"), "11:22:33");
}

TEST_F(FormatterTest, AddDateTimeFromString) {
    formatter.addParameter("DateTime", "2007-01-20 11:22:33", DataInfo::DateTime);
    formatter.addParameter("Date", "2007-01-20", DataInfo::Date);
    formatter.addParameter("Time", "11:22:33", DataInfo::Time);

    // Should be validated and reformatted
    EXPECT_EQ(formatter.getValueOrThrow("DateTime"), "2007-01-20 11:22:33");
    EXPECT_EQ(formatter.getValueOrThrow("Date"), "2007-01-20");
    EXPECT_EQ(formatter.getValueOrThrow("Time"), "11:22:33");
}

TEST_F(FormatterTest, ThrowOnInvalidDateTimeString) {
    EXPECT_THROW(
        formatter.addParameter("Bad", "invalid-date", DataInfo::DateTime),
        FormatterException
        );
}

TEST_F(FormatterTest, ThrowOnWrongTypeForChrono) {
    auto time = timeFormatter::now();

    EXPECT_THROW(
        formatter.addParameter("Bad", time, DataInfo::Int),
        FormatterException
        );
}

// ============================================================================
// GetInfo Tests
// ============================================================================

TEST_F(FormatterTest, GetInfoSuccess) {
    formatter.addParameter("Height", FormatterValue{175});

    auto info = formatter.getInfo("Height");
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->param, "Height");
    EXPECT_EQ(info->value, "175");
    EXPECT_EQ(info->type, DataInfo::Int);
}

TEST_F(FormatterTest, GetInfoNotFound) {
    auto info = formatter.getInfo("NotExists");
    EXPECT_FALSE(info.has_value());
}

TEST_F(FormatterTest, GetInfoOrThrow) {
    formatter.addParameter("Name", FormatterValue{std::string("John")});

    const auto& info = formatter.getInfoOrThrow("Name");
    EXPECT_EQ(info.param, "Name");
    EXPECT_EQ(info.value, "John");
    EXPECT_EQ(info.type, DataInfo::String);
}

TEST_F(FormatterTest, GetInfoOrThrowNotFound) {
    EXPECT_THROW(
        formatter.getInfoOrThrow("NotExists"),
        FormatterException
        );
}

// ============================================================================
// GetAsTime Tests
// ============================================================================

TEST_F(FormatterTest, GetAsTimeSuccess) {
    formatter.addParameter("DateTime", "2007-01-20 11:22:33", DataInfo::DateTime);

    EXPECT_NO_THROW(formatter.getAsTime("DateTime"));
}

TEST_F(FormatterTest, GetAsTimeNotFound) {
    EXPECT_THROW(
        formatter.getAsTime("NotExists"),
        FormatterException
        );
}

TEST_F(FormatterTest, GetAsTimeWrongType) {
    formatter.addParameter("Height", FormatterValue{175});

    EXPECT_THROW(
        formatter.getAsTime("Height"),
        FormatterException
        );
}

TEST_F(FormatterTest, GetAsTimeAllDateTimeTypes) {
    formatter.addParameter("DateTime", "2007-01-20 11:22:33", DataInfo::DateTime);
    formatter.addParameter("DateTimeNoSec", "2007-01-20 11:22", DataInfo::DateTimeNoSec);
    formatter.addParameter("Date", "2007-01-20", DataInfo::Date);
    formatter.addParameter("Time", "11:22:33", DataInfo::Time);

    EXPECT_NO_THROW(formatter.getAsTime("DateTime"));
    EXPECT_NO_THROW(formatter.getAsTime("DateTimeNoSec"));
    EXPECT_NO_THROW(formatter.getAsTime("Date"));
    EXPECT_NO_THROW(formatter.getAsTime("Time"));
}

// ============================================================================
// ToMap Tests
// ============================================================================

TEST_F(FormatterTest, ToMapAllTypes) {
    auto timeStr = "2007-01-20 11:22:33";
    auto time = timeFormatter::fromString(timeStr, DataInfo::DateTime);

    formatter.addParameter("Int", FormatterValue{10});
    formatter.addParameter("Double", FormatterValue{11.11});
    formatter.addParameter("String", FormatterValue{std::string("RandomString")});
    formatter.addParameter("Bool", FormatterValue{true});
    formatter.addParameter("DateTime", time, DataInfo::DateTime);

    auto map = formatter.toMap();

    EXPECT_EQ(map.size(), 5);
    EXPECT_EQ(map["Int"], "10");
    EXPECT_EQ(map["String"], "RandomString");
    EXPECT_EQ(map["Bool"], "1");
    EXPECT_EQ(map["DateTime"], "2007-01-20 11:22:33");
}

// ============================================================================
// Parameters Access Tests
// ============================================================================

TEST_F(FormatterTest, ParametersReadOnlyAccess) {
    formatter.addParameter("Param1", FormatterValue{1});
    formatter.addParameter("Param2", FormatterValue{2});

    const auto& params = formatter.parameters();

    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0].param, "Param1");
    EXPECT_EQ(params[1].param, "Param2");
}

// ============================================================================
// Performance / Edge Cases
// ============================================================================

TEST(FormatterEdgeCaseTest, ManyParameters) {
    TypeToStringFormatter formatter;

    for (int i = 0; i < 1000; ++i) {
        formatter.addParameter(std::format("Param{}", i), FormatterValue{i});
    }

    EXPECT_EQ(formatter.size(), 1000);

    // O(1) lookup should be fast
    for (int i = 0; i < 1000; i += 100) {
        auto name = std::format("Param{}", i);
        EXPECT_TRUE(formatter.contains(name));
    }
}

TEST(FormatterEdgeCaseTest, EmptyParameterName) {
    TypeToStringFormatter formatter;
    formatter.addParameter("", FormatterValue{42});

    EXPECT_TRUE(formatter.contains(""));
    EXPECT_EQ(formatter.getValueOrThrow(""), "42");
}

TEST(FormatterEdgeCaseTest, SpecialCharactersInName) {
    TypeToStringFormatter formatter;
    formatter.addParameter("special:name-with.chars", FormatterValue{100});

    EXPECT_EQ(formatter.getValueOrThrow("special:name-with.chars"), "100");
}

TEST(FormatterEdgeCaseTest, DuplicateParameterNames) {
    TypeToStringFormatter formatter;
    formatter.addParameter("Duplicate", FormatterValue{1});
    formatter.addParameter("Duplicate", FormatterValue{2});

    // Last write wins - both are added to the list
    EXPECT_EQ(formatter.size(), 2);
    // But lookup returns the last one added (due to map overwrite)
    auto value = formatter.getValueOrThrow("Duplicate");
    EXPECT_EQ(value, "2");
}

// ============================================================================
// Copy and Move Semantics Tests
// ============================================================================

TEST(FormatterCopyMoveTest, CopyConstructor) {
    TypeToStringFormatter original;
    original.addParameter("Int", FormatterValue{42});
    original.addParameter("String", FormatterValue{std::string("test")});

    TypeToStringFormatter copy(original);

    EXPECT_EQ(copy.size(), 2);
    EXPECT_EQ(copy.getValueOrThrow("Int"), "42");
    EXPECT_EQ(copy.getValueOrThrow("String"), "test");

    // Ensure deep copy - modifying copy shouldn't affect original
    copy.addParameter("New", FormatterValue{100});
    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(original.size(), 2);
}

TEST(FormatterCopyMoveTest, CopyAssignment) {
    TypeToStringFormatter original;
    original.addParameter("Value", FormatterValue{123});

    TypeToStringFormatter copy;
    copy.addParameter("Other", FormatterValue{999});

    copy = original;

    EXPECT_EQ(copy.size(), 1);
    EXPECT_EQ(copy.getValueOrThrow("Value"), "123");
    EXPECT_FALSE(copy.contains("Other"));
}

TEST(FormatterCopyMoveTest, MoveConstructor) {
    TypeToStringFormatter original;
    original.addParameter("Int", FormatterValue{42});
    original.addParameter("String", FormatterValue{std::string("test")});

    TypeToStringFormatter moved(std::move(original));

    EXPECT_EQ(moved.size(), 2);
    EXPECT_EQ(moved.getValueOrThrow("Int"), "42");
    EXPECT_EQ(moved.getValueOrThrow("String"), "test");

    // Original should be in valid but unspecified state
    // We can only safely check it can be used again
    EXPECT_NO_THROW(original.clear());
}

TEST(FormatterCopyMoveTest, MoveAssignment) {
    TypeToStringFormatter original;
    original.addParameter("Value", FormatterValue{456});

    TypeToStringFormatter moved;
    moved.addParameter("Other", FormatterValue{999});

    moved = std::move(original);

    EXPECT_EQ(moved.size(), 1);
    EXPECT_EQ(moved.getValueOrThrow("Value"), "456");

    // Original should be in valid but unspecified state
    EXPECT_NO_THROW(original.clear());
}

TEST(FormatterCopyMoveTest, SelfAssignment) {
    TypeToStringFormatter formatter;
    formatter.addParameter("Test", FormatterValue{789});

    // Self-assignment should be safe
    formatter = formatter;

    EXPECT_EQ(formatter.size(), 1);
    EXPECT_EQ(formatter.getValueOrThrow("Test"), "789");
}
