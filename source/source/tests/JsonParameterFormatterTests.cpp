#include "../JsonParameterFormatter.h"
#include "gtest/gtest.h"

// ============================================================================
// Static Utility Tests (JsonParameterFormatter static methods)
// ============================================================================

TEST(JsonFormatterTest, FromJsonSimple) {
    std::string json = R"({"name":"John","age":"30"})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params["name"], "John");
    EXPECT_EQ(params["age"], "30");
}

TEST(JsonFormatterTest, FromJsonWithNumbers) {
    std::string json = R"({"name":"John","age":30,"height":175.5})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["name"], "John");
    EXPECT_EQ(params["age"], "30");
    EXPECT_TRUE(params["height"].find("175.5") != std::string::npos);
}

TEST(JsonFormatterTest, FromJsonWithBool) {
    std::string json = R"({"active":true,"deleted":false})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["active"], "true");
    EXPECT_EQ(params["deleted"], "false");
}

TEST(JsonFormatterTest, FromJsonWithNull) {
    std::string json = R"({"value":null})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["value"], "NULL");
}

TEST(JsonFormatterTest, FromJsonInvalid) {
    std::string json = R"({"name": "John", "age":})";
    
    EXPECT_THROW(
        JsonParameterFormatter::fromJsonString(json),
        JsonFormatterException
    );
}

TEST(JsonFormatterTest, FromJsonNotObject) {
    std::string json = R"(["array", "values"])";
    
    EXPECT_THROW(
        JsonParameterFormatter::fromJsonString(json),
        JsonFormatterException
    );
}

TEST(JsonFormatterTest, TryFromJsonSuccess) {
    std::string json = R"({"name":"John"})";
    
    auto result = JsonParameterFormatter::tryFromJsonString(json);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ((*result)["name"], "John");
}

TEST(JsonFormatterTest, TryFromJsonFailure) {
    std::string json = R"(invalid json)";
    
    auto result = JsonParameterFormatter::tryFromJsonString(json);
    
    EXPECT_FALSE(result.has_value());
}

TEST(JsonFormatterTest, ToJsonSimple) {
    std::map<std::string, std::string> params = {
        {"name", "John"},
        {"age", "30"}
    };
    
    auto json = JsonParameterFormatter::toJsonString(params);
    
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("name") != std::string::npos);
    EXPECT_TRUE(json.find("John") != std::string::npos);
}

TEST(JsonFormatterTest, ToJsonPretty) {
    std::map<std::string, std::string> params = {
        {"name", "John"},
        {"age", "30"}
    };
    
    auto json = JsonParameterFormatter::toJsonString(params, true);
    
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("\n") != std::string::npos);  // Has newlines
    EXPECT_TRUE(json.find("    ") != std::string::npos);  // Has indentation
}

TEST(JsonFormatterTest, ToJsonFromFormatter) {
    TypeToStringFormatter formatter;
    auto timePoint = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);
    
    formatter.addParameter("DateTime", timePoint, DataInfo::DateTime);
    formatter.addParameter("Name", FormatterValue{std::string("John")});
    
    auto json = JsonParameterFormatter::toJsonString(formatter);
    
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("DateTime") != std::string::npos);
    EXPECT_TRUE(json.find("2007-01-20 11:22:33") != std::string::npos);
}

TEST(JsonFormatterTest, IsValid) {
    EXPECT_TRUE(JsonParameterFormatter::isValidJson(R"({"name":"John"})"));
    EXPECT_FALSE(JsonParameterFormatter::isValidJson(R"(invalid)"));
    EXPECT_FALSE(JsonParameterFormatter::isValidJson(R"(["array"])"));
}

TEST(JsonFormatterTest, GetParseError) {
    std::string badJson = R"({"name": "John", "age":})";
    
    auto error = JsonParameterFormatter::getParseError(badJson);
    
    EXPECT_FALSE(error.empty());
    EXPECT_TRUE(error.find("byte") != std::string::npos);
}

TEST(JsonFormatterTest, RoundTripConversion) {
    TypeToStringFormatter formatter;
    auto timePoint = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);
    
    formatter.addParameter("DateTime", timePoint, DataInfo::DateTime);
    formatter.addParameter("Date", timePoint, DataInfo::Date);
    formatter.addParameter("Time", timePoint, DataInfo::Time);
    
    // Convert to JSON
    auto json = JsonParameterFormatter::toJsonString(formatter);
    EXPECT_FALSE(json.empty());
    
    // Parse back
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["DateTime"], "2007-01-20 11:22:33");
    EXPECT_EQ(params["Date"], "2007-01-20");
    EXPECT_EQ(params["Time"], "11:22:33");
}

// ============================================================================
// Instance API Tests (::JsonParameterFormatter)
// ============================================================================

TEST(JsonParameterFormatterInstanceTest, AddParametersAndSerialize) {
    ::JsonParameterFormatter formatter;
    
    formatter.addParameter("Name", FormatterValue{std::string("John")});
    formatter.addParameter("Age", FormatterValue{30});
    formatter.addParameter("Height", FormatterValue{175.5});
    formatter.addParameter("Active", FormatterValue{true});
    
    auto json = formatter.toJson();
    
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("Name") != std::string::npos);
    EXPECT_TRUE(json.find("John") != std::string::npos);
    EXPECT_TRUE(json.find("Age") != std::string::npos);
}

TEST(JsonParameterFormatterInstanceTest, AddParametersAndSerializePretty) {
    ::JsonParameterFormatter formatter;
    
    formatter.addParameter("Name", FormatterValue{std::string("John")});
    formatter.addParameter("Age", FormatterValue{30});
    
    auto json = formatter.toJson(true);
    
    EXPECT_TRUE(json.find("\n") != std::string::npos);
    EXPECT_TRUE(json.find("    ") != std::string::npos);
}

TEST(JsonParameterFormatterInstanceTest, LoadFromJson) {
    ::JsonParameterFormatter formatter;
    
    std::string json = R"({"Name":"John","Age":"30"})";
    formatter.fromJson(json);
    
    EXPECT_TRUE(formatter.contains("Name"));
    EXPECT_TRUE(formatter.contains("Age"));
    EXPECT_EQ(formatter.getValueOrThrow("Name"), "John");
    EXPECT_EQ(formatter.getValueOrThrow("Age"), "30");
}

TEST(JsonParameterFormatterInstanceTest, TryFromJsonSuccess) {
    ::JsonParameterFormatter formatter;
    
    std::string json = R"({"Name":"John"})";
    bool result = formatter.tryFromJson(json);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(formatter.contains("Name"));
    EXPECT_EQ(formatter.getValueOrThrow("Name"), "John");
}

TEST(JsonParameterFormatterInstanceTest, TryFromJsonFailure) {
    ::JsonParameterFormatter formatter;
    
    std::string json = R"(invalid json)";
    bool result = formatter.tryFromJson(json);
    
    EXPECT_FALSE(result);
}

TEST(JsonParameterFormatterInstanceTest, AddDateTimeParameters) {
    ::JsonParameterFormatter formatter;
    auto timePoint = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);
    
    formatter.addParameter("DateTime", timePoint, DataInfo::DateTime);
    formatter.addParameter("Date", "2007-01-20", DataInfo::Date);
    formatter.addParameter("Time", "11:22:33", DataInfo::Time);
    
    auto json = formatter.toJson();
    
    EXPECT_TRUE(json.find("2007-01-20 11:22:33") != std::string::npos);
    EXPECT_TRUE(json.find("2007-01-20") != std::string::npos);
    EXPECT_TRUE(json.find("11:22:33") != std::string::npos);
}

TEST(JsonParameterFormatterInstanceTest, GetParameters) {
    ::JsonParameterFormatter formatter;
    
    formatter.addParameter("Param1", FormatterValue{std::string("Value1")});
    formatter.addParameter("Param2", FormatterValue{100});
    
    auto params = formatter.getParameters();
    
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params["Param1"], "Value1");
    EXPECT_EQ(params["Param2"], "100");
}

TEST(JsonParameterFormatterInstanceTest, RoundTripWithInstanceMethods) {
    ::JsonParameterFormatter formatter;
    auto timePoint = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);
    
    formatter.addParameter("DateTime", timePoint, DataInfo::DateTime);
    formatter.addParameter("Date", timePoint, DataInfo::Date);
    formatter.addParameter("Name", FormatterValue{std::string("TestUser")});
    
    // Serialize
    std::string json = formatter.toJson();
    EXPECT_FALSE(json.empty());
    
    // Deserialize into new formatter
    ::JsonParameterFormatter formatter2;
    formatter2.fromJson(json);
    
    EXPECT_EQ(formatter2.getValueOrThrow("DateTime"), "2007-01-20 11:22:33");
    EXPECT_EQ(formatter2.getValueOrThrow("Date"), "2007-01-20");
    EXPECT_EQ(formatter2.getValueOrThrow("Name"), "TestUser");
}

TEST(JsonParameterFormatterInstanceTest, ClearAndReload) {
    ::JsonParameterFormatter formatter;
    
    formatter.addParameter("Param1", FormatterValue{std::string("Value1")});
    EXPECT_EQ(formatter.size(), 1);
    
    std::string json = R"({"Param2":"Value2","Param3":"Value3"})";
    formatter.fromJson(json);  // Should clear existing
    
    EXPECT_EQ(formatter.size(), 2);
    EXPECT_FALSE(formatter.contains("Param1"));
    EXPECT_TRUE(formatter.contains("Param2"));
    EXPECT_TRUE(formatter.contains("Param3"));
}

TEST(JsonParameterFormatterInstanceTest, InheritedMethods) {
    ::JsonParameterFormatter formatter;
    
    // Test inherited TypeToStringFormatter methods
    formatter.addParameter("Test", FormatterValue{42});
    
    EXPECT_EQ(formatter.size(), 1);
    EXPECT_TRUE(formatter.contains("Test"));
    EXPECT_EQ(formatter.getValueOrThrow("Test"), "42");
    
    formatter.clear();
    EXPECT_EQ(formatter.size(), 0);
}

TEST(JsonParameterFormatterInstanceTest, StaticUtilitiesFromInstance) {
    // Test static utility methods
    std::map<std::string, std::string> params = {
        {"key1", "value1"},
        {"key2", "value2"}
    };
    
    auto json = ::JsonParameterFormatter::toJsonString(params);
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("key1") != std::string::npos);
    
    auto parsedParams = ::JsonParameterFormatter::fromJsonString(json);
    EXPECT_EQ(parsedParams.size(), 2);
    EXPECT_EQ(parsedParams["key1"], "value1");
    
    EXPECT_TRUE(::JsonParameterFormatter::isValidJson(json));
    EXPECT_FALSE(::JsonParameterFormatter::isValidJson("invalid"));
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST(JsonEdgeCaseTest, EmptyJson) {
    std::string json = R"({})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params.empty());
}

TEST(JsonEdgeCaseTest, JsonWithSpecialCharacters) {
    std::string json = R"({"special:key":"value-with-dashes","unicode":"こんにちは"})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["special:key"], "value-with-dashes");
    EXPECT_EQ(params["unicode"], "こんにちは");
}

TEST(JsonEdgeCaseTest, JsonWithEscapedCharacters) {
    std::string json = R"({"path":"C:\\Users\\Test","quote":"He said \"Hello\""})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params["path"].find("Users") != std::string::npos);
    EXPECT_TRUE(params["quote"].find("Hello") != std::string::npos);
}

TEST(JsonEdgeCaseTest, JsonWithNestedObject) {
    // Nested objects should be serialized as strings
    std::string json = R"({"nested":{"inner":"value"}})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params.contains("nested"));
    // Should contain the serialized nested object
    EXPECT_TRUE(params["nested"].find("inner") != std::string::npos);
}

TEST(JsonEdgeCaseTest, JsonWithArray) {
    std::string json = R"({"items":[1,2,3]})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params.contains("items"));
    // Array should be serialized as string
    EXPECT_TRUE(params["items"].find("[") != std::string::npos);
}

TEST(JsonEdgeCaseTest, VeryLargeNumber) {
    std::string json = R"({"bignum":999999999999999999})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params["bignum"].find("999999999999999999") != std::string::npos);
}

TEST(JsonEdgeCaseTest, NegativeNumbers) {
    std::string json = R"({"negative":-42,"float":-3.14})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_TRUE(params["negative"].find("-42") != std::string::npos);
    EXPECT_TRUE(params["float"].find("-3.14") != std::string::npos);
}

TEST(JsonEdgeCaseTest, EmptyString) {
    std::string json = R"({"empty":""})";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params["empty"], "");
}

TEST(JsonEdgeCaseTest, ParseErrorDetails) {
    std::string badJson = R"({"unclosed": "string)";
    
    auto error = JsonParameterFormatter::getParseError(badJson);
    
    EXPECT_FALSE(error.empty());
    EXPECT_TRUE(error.find("byte") != std::string::npos);
    // Should show context around error
    EXPECT_TRUE(error.find("Near:") != std::string::npos);
}

TEST(JsonEdgeCaseTest, MultilineJson) {
    std::string json = R"({
        "name": "John",
        "age": 30,
        "city": "New York"
    })";
    
    auto params = JsonParameterFormatter::fromJsonString(json);
    
    EXPECT_EQ(params.size(), 3);
    EXPECT_EQ(params["name"], "John");
}

// ============================================================================
// Copy and Move Semantics Tests
// ============================================================================

TEST(JsonFormatterCopyMoveTest, CopyConstructor) {
    ::JsonParameterFormatter original;
    auto timePoint = timeFormatter::fromString("2007-01-20 11:22:33", DataInfo::DateTime);
    
    original.addParameter("DateTime", timePoint, DataInfo::DateTime);
    original.addParameter("Name", FormatterValue{std::string("John")});
    original.addParameter("Age", FormatterValue{30});
    
    ::JsonParameterFormatter copy(original);
    
    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(copy.getValueOrThrow("Name"), "John");
    EXPECT_EQ(copy.getValueOrThrow("Age"), "30");
    EXPECT_EQ(copy.getValueOrThrow("DateTime"), "2007-01-20 11:22:33");
    
    // Verify JSON serialization works on copy
    auto json = copy.toJson();
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("John") != std::string::npos);
}

TEST(JsonFormatterCopyMoveTest, CopyAssignment) {
    ::JsonParameterFormatter original;
    original.addParameter("Value", FormatterValue{123});
    
    ::JsonParameterFormatter copy;
    copy.addParameter("Other", FormatterValue{999});
    
    copy = original;
    
    EXPECT_EQ(copy.size(), 1);
    EXPECT_EQ(copy.getValueOrThrow("Value"), "123");
    EXPECT_FALSE(copy.contains("Other"));
    
    // Verify JSON methods still work
    auto json = copy.toJson();
    EXPECT_TRUE(json.find("Value") != std::string::npos);
}

TEST(JsonFormatterCopyMoveTest, MoveConstructor) {
    ::JsonParameterFormatter original;
    original.addParameter("Name", FormatterValue{std::string("Test")});
    original.addParameter("Value", FormatterValue{42});
    
    ::JsonParameterFormatter moved(std::move(original));
    
    EXPECT_EQ(moved.size(), 2);
    EXPECT_EQ(moved.getValueOrThrow("Name"), "Test");
    EXPECT_EQ(moved.getValueOrThrow("Value"), "42");
    
    // Verify JSON serialization works on moved object
    auto json = moved.toJson();
    EXPECT_FALSE(json.empty());
    EXPECT_TRUE(json.find("Test") != std::string::npos);
}

TEST(JsonFormatterCopyMoveTest, MoveAssignment) {
    ::JsonParameterFormatter original;
    original.addParameter("Value", FormatterValue{456});
    
    ::JsonParameterFormatter moved;
    moved.addParameter("Other", FormatterValue{999});
    
    moved = std::move(original);
    
    EXPECT_EQ(moved.size(), 1);
    EXPECT_EQ(moved.getValueOrThrow("Value"), "456");
    
    // Verify JSON methods work after move
    auto json = moved.toJson();
    EXPECT_TRUE(json.find("Value") != std::string::npos);
}

TEST(JsonFormatterCopyMoveTest, CopyThenSerialize) {
    ::JsonParameterFormatter original;
    original.fromJson(R"({"Key1":"Value1","Key2":"Value2"})");
    
    ::JsonParameterFormatter copy(original);
    
    // Both should produce the same JSON
    auto originalJson = original.toJson();
    auto copyJson = copy.toJson();
    
    // Parse both to compare content (order may differ)
    auto originalParams = JsonParameterFormatter::fromJsonString(originalJson);
    auto copyParams = JsonParameterFormatter::fromJsonString(copyJson);
    
    EXPECT_EQ(originalParams.size(), copyParams.size());
    EXPECT_EQ(originalParams["Key1"], copyParams["Key1"]);
    EXPECT_EQ(originalParams["Key2"], copyParams["Key2"]);
}

TEST(JsonFormatterCopyMoveTest, DeserializeAfterCopy) {
    ::JsonParameterFormatter original;
    original.addParameter("Original", FormatterValue{100});
    
    ::JsonParameterFormatter copy(original);
    
    // Deserialize new data into copy
    copy.fromJson(R"({"New":"Data"})");
    
    // Original should be unchanged
    EXPECT_EQ(original.size(), 1);
    EXPECT_TRUE(original.contains("Original"));
    
    // Copy should have new data
    EXPECT_EQ(copy.size(), 1);
    EXPECT_TRUE(copy.contains("New"));
    EXPECT_FALSE(copy.contains("Original"));
}
