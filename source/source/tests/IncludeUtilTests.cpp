#include <gtest/gtest.h>
#include "../include_util.h"
#include <filesystem>
#include <stdexcept>

// ============================================================================
// Trimmer Tests
// ============================================================================

TEST(TrimmerTest, RtrimRemovesTrailingWhitespace) {
    EXPECT_EQ(Trimmer::rtrim("hello   "), "hello");
    EXPECT_EQ(Trimmer::rtrim("hello\t\n\r"), "hello");
    EXPECT_EQ(Trimmer::rtrim("hello world  "), "hello world");
    EXPECT_EQ(Trimmer::rtrim("  hello  "), "  hello");
}

TEST(TrimmerTest, RtrimWithCustomCharacters) {
    EXPECT_EQ(Trimmer::rtrim("hello***", "*"), "hello");
    EXPECT_EQ(Trimmer::rtrim("hello.,.", ",."), "hello");
    EXPECT_EQ(Trimmer::rtrim("test123", "123"), "test");
}

TEST(TrimmerTest, RtrimEmptyAndAllWhitespace) {
    EXPECT_EQ(Trimmer::rtrim(""), "");
    EXPECT_EQ(Trimmer::rtrim("   "), "");
    EXPECT_EQ(Trimmer::rtrim("\t\n\r"), "");
}

TEST(TrimmerTest, LtrimRemovesLeadingWhitespace) {
    EXPECT_EQ(Trimmer::ltrim("   hello"), "hello");
    EXPECT_EQ(Trimmer::ltrim("\t\n\rhello"), "hello");
    EXPECT_EQ(Trimmer::ltrim("  hello world"), "hello world");
    EXPECT_EQ(Trimmer::ltrim("  hello  "), "hello  ");
}

TEST(TrimmerTest, LtrimWithCustomCharacters) {
    EXPECT_EQ(Trimmer::ltrim("***hello", "*"), "hello");
    EXPECT_EQ(Trimmer::ltrim(".,hello", ",."), "hello");
    EXPECT_EQ(Trimmer::ltrim("123test", "123"), "test");
}

TEST(TrimmerTest, LtrimEmptyAndAllWhitespace) {
    EXPECT_EQ(Trimmer::ltrim(""), "");
    EXPECT_EQ(Trimmer::ltrim("   "), "");
    EXPECT_EQ(Trimmer::ltrim("\t\n\r"), "");
}

TEST(TrimmerTest, TrimRemovesBothEnds) {
    EXPECT_EQ(Trimmer::trim("  hello  "), "hello");
    EXPECT_EQ(Trimmer::trim("\t\nhello\r\n"), "hello");
    EXPECT_EQ(Trimmer::trim("  hello world  "), "hello world");
    EXPECT_EQ(Trimmer::trim("hello"), "hello");
}

TEST(TrimmerTest, TrimWithCustomCharacters) {
    EXPECT_EQ(Trimmer::trim("***hello***", "*"), "hello");
    EXPECT_EQ(Trimmer::trim(".,hello.,", ",."), "hello");
    EXPECT_EQ(Trimmer::trim("123test321", "123"), "test");
}

TEST(TrimmerTest, TrimEmptyAndAllWhitespace) {
    EXPECT_EQ(Trimmer::trim(""), "");
    EXPECT_EQ(Trimmer::trim("   "), "");
    EXPECT_EQ(Trimmer::trim("\t\n\r \f\v"), "");
}

TEST(TrimmerTest, TrimPreservesInternalWhitespace) {
    EXPECT_EQ(Trimmer::trim("  hello  world  "), "hello  world");
    EXPECT_EQ(Trimmer::trim("  a\tb\nc  "), "a\tb\nc");
}

TEST(TrimmerTest, RtrimInPlaceModifiesString) {
    std::string str = "hello   ";
    Trimmer::rtrimInPlace(str);
    EXPECT_EQ(str, "hello");
    
    str = "test\t\n";
    Trimmer::rtrimInPlace(str);
    EXPECT_EQ(str, "test");
}

TEST(TrimmerTest, LtrimInPlaceModifiesString) {
    std::string str = "   hello";
    Trimmer::ltrimInPlace(str);
    EXPECT_EQ(str, "hello");
    
    str = "\t\ntest";
    Trimmer::ltrimInPlace(str);
    EXPECT_EQ(str, "test");
}

TEST(TrimmerTest, TrimInPlaceModifiesString) {
    std::string str = "  hello  ";
    Trimmer::trimInPlace(str);
    EXPECT_EQ(str, "hello");
    
    str = "\t\ntest\r\n";
    Trimmer::trimInPlace(str);
    EXPECT_EQ(str, "test");
}

TEST(TrimmerTest, InPlaceReturnsReference) {
    std::string str = "  hello  ";
    std::string& result = Trimmer::trimInPlace(str);
    EXPECT_EQ(&result, &str);  // Same address
    EXPECT_EQ(result, "hello");
}

// ============================================================================
// CommonUtil::sqlRowOffset Tests
// ============================================================================

TEST(SqlRowOffsetTest, BasicPagination) {
    std::uint32_t currentPage, pageCount;
    
    // Page 1 of 100 items, 10 per page
    auto offset = CommonUtil::sqlRowOffset(1, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 1);
    EXPECT_EQ(pageCount, 10);
    
    // Page 2
    offset = CommonUtil::sqlRowOffset(2, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 10);
    EXPECT_EQ(currentPage, 2);
    EXPECT_EQ(pageCount, 10);
    
    // Page 5
    offset = CommonUtil::sqlRowOffset(5, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 40);
    EXPECT_EQ(currentPage, 5);
    EXPECT_EQ(pageCount, 10);
}

TEST(SqlRowOffsetTest, LastPagePartialResults) {
    std::uint32_t currentPage, pageCount;
    
    // 95 items with 10 per page = 10 pages (last has 5 items)
    auto offset = CommonUtil::sqlRowOffset(10, 10, 95, currentPage, pageCount);
    EXPECT_EQ(offset, 90);
    EXPECT_EQ(currentPage, 10);
    EXPECT_EQ(pageCount, 10);
}

TEST(SqlRowOffsetTest, ExactMultipleOfPageSize) {
    std::uint32_t currentPage, pageCount;
    
    // Exactly 100 items, 10 per page
    auto offset = CommonUtil::sqlRowOffset(10, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 90);
    EXPECT_EQ(currentPage, 10);
    EXPECT_EQ(pageCount, 10);
}

TEST(SqlRowOffsetTest, LimitGreaterThanTotal) {
    std::uint32_t currentPage, pageCount;
    
    // Limit is larger than total count
    auto offset = CommonUtil::sqlRowOffset(1, 100, 50, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 0);
    EXPECT_EQ(pageCount, 0);
}

TEST(SqlRowOffsetTest, LimitEqualsTotal) {
    std::uint32_t currentPage, pageCount;
    
    // Limit equals total count
    auto offset = CommonUtil::sqlRowOffset(1, 50, 50, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 0);
    EXPECT_EQ(pageCount, 0);
}

TEST(SqlRowOffsetTest, InvalidPageNumberZero) {
    std::uint32_t currentPage, pageCount;
    
    // Page 0 is invalid
    auto offset = CommonUtil::sqlRowOffset(0, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 0);
}

TEST(SqlRowOffsetTest, PageNumberExceedsTotal) {
    std::uint32_t currentPage, pageCount;
    
    // Requesting page 20 when only 10 pages exist
    auto offset = CommonUtil::sqlRowOffset(20, 10, 100, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 0);
}

TEST(SqlRowOffsetTest, ZeroLimitThrowsException) {
    std::uint32_t currentPage, pageCount;
    
    EXPECT_THROW(
        CommonUtil::sqlRowOffset(1, 0, 100, currentPage, pageCount),
        std::invalid_argument
    );
}

TEST(SqlRowOffsetTest, SimplifiedOverload) {
    // Without output parameters
    EXPECT_EQ(CommonUtil::sqlRowOffset(1, 10, 100), 0);
    EXPECT_EQ(CommonUtil::sqlRowOffset(2, 10, 100), 10);
    EXPECT_EQ(CommonUtil::sqlRowOffset(5, 10, 100), 40);
    EXPECT_EQ(CommonUtil::sqlRowOffset(10, 10, 100), 90);
}

TEST(SqlRowOffsetTest, SimplifiedOverloadInvalidCases) {
    EXPECT_EQ(CommonUtil::sqlRowOffset(0, 10, 100), 0);
    EXPECT_EQ(CommonUtil::sqlRowOffset(20, 10, 100), 0);
    EXPECT_EQ(CommonUtil::sqlRowOffset(1, 100, 50), 0);
}

TEST(SqlRowOffsetTest, SimplifiedOverloadZeroLimitThrows) {
    EXPECT_THROW(CommonUtil::sqlRowOffset(1, 0, 100), std::invalid_argument);
}

TEST(SqlRowOffsetTest, EdgeCaseOneItem) {
    std::uint32_t currentPage, pageCount;
    
    auto offset = CommonUtil::sqlRowOffset(1, 1, 10, currentPage, pageCount);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(currentPage, 1);
    EXPECT_EQ(pageCount, 10);
}

TEST(SqlRowOffsetTest, EdgeCaseLargeNumbers) {
    std::uint32_t currentPage, pageCount;
    
    // Large dataset: 1,000,000 items, 1000 per page
    auto offset = CommonUtil::sqlRowOffset(500, 1000, 1000000, currentPage, pageCount);
    EXPECT_EQ(offset, 499000);
    EXPECT_EQ(currentPage, 500);
    EXPECT_EQ(pageCount, 1000);
}

TEST(SqlRowOffsetTest, SequentialPaginationScenario) {
    // Test from existing UtilFunctionsTests - 100 items, 30 per page = 4 pages
    std::uint32_t page = 1;
    std::uint32_t limitCount = 30;
    std::uint32_t totalCount = 100;
    std::uint32_t realCurrentPage = 0;
    std::uint32_t pageCount = 0;
    
    // Page 1: offset 0, should have 4 pages total
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);
    
    // Page 2: offset 30
    page = 2;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 30);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);
    
    // Page 3: offset 60
    page = 3;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 60);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);
    
    // Page 4: offset 90 (last page with 10 items)
    page = 4;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 90);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);
    
    // Page 5: invalid, should return 0
    page = 5;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 4);
    
    // Change limit to 100 (larger than total): should return 0 with pageCount 0
    page = 2;
    limitCount = 100;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 0);
    
    // Change limit to 300 (much larger): should still return 0
    page = 20;
    limitCount = 300;
    EXPECT_EQ(CommonUtil::sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 0);
}

// ============================================================================
// StdBinary Tests
// ============================================================================

class StdBinaryTest : public ::testing::Test {
protected:
    std::filesystem::path testDir;
    std::filesystem::path testFile;
    
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "include_util_v2_tests";
        std::filesystem::create_directories(testDir);
        testFile = testDir / "test_file.bin";
    }
    
    void TearDown() override {
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }
};

TEST_F(StdBinaryTest, WriteAndReadTextData) {
    std::string testData = "Hello, World!";
    
    StdBinary::toBinary(testFile, testData);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_EQ(result, testData);
}

TEST_F(StdBinaryTest, WriteAndReadBinaryData) {
    std::string binaryData = "\x00\x01\x02\x03\xFF\xFE\xFD";
    
    StdBinary::toBinary(testFile, binaryData);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_EQ(result, binaryData);
}

TEST_F(StdBinaryTest, WriteEmptyData) {
    std::string emptyData;
    
    StdBinary::toBinary(testFile, emptyData);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(StdBinaryTest, WriteLargeData) {
    std::string largeData(100000, 'A');
    
    StdBinary::toBinary(testFile, largeData);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_EQ(result.size(), 100000);
    EXPECT_EQ(result, largeData);
}

TEST_F(StdBinaryTest, OverwriteExistingFile) {
    StdBinary::toBinary(testFile, "first");
    StdBinary::toBinary(testFile, "second");
    
    auto result = StdBinary::toStdString(testFile);
    EXPECT_EQ(result, "second");
}

TEST_F(StdBinaryTest, ReadNonExistentFileThrows) {
    auto nonExistentFile = testDir / "does_not_exist.bin";
    
    EXPECT_THROW(
        StdBinary::toStdString(nonExistentFile),
        std::system_error
    );
}

TEST_F(StdBinaryTest, WriteToInvalidPathThrows) {
    auto invalidPath = std::filesystem::path("/invalid/path/that/does/not/exist/file.bin");
    
    EXPECT_THROW(
        StdBinary::toBinary(invalidPath, "data"),
        std::system_error
    );
}

TEST_F(StdBinaryTest, WriteMultilineData) {
    std::string multiline = "Line 1\nLine 2\r\nLine 3\n";
    
    StdBinary::toBinary(testFile, multiline);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_EQ(result, multiline);
}

TEST_F(StdBinaryTest, WriteUnicodeData) {
    std::string unicode = "Hello 世界 🌍";
    
    StdBinary::toBinary(testFile, unicode);
    auto result = StdBinary::toStdString(testFile);
    
    EXPECT_EQ(result, unicode);
}

TEST_F(StdBinaryTest, FileExistsAfterWrite) {
    StdBinary::toBinary(testFile, "test");
    EXPECT_TRUE(std::filesystem::exists(testFile));
}

TEST_F(StdBinaryTest, FileSizeMatchesData) {
    std::string data = "test data with 30 characters!";
    
    StdBinary::toBinary(testFile, data);
    auto fileSize = std::filesystem::file_size(testFile);
    
    EXPECT_EQ(fileSize, data.size());
}

// ============================================================================
// DataInfo Structure Tests
// ============================================================================

TEST(DataInfoTest, StructureInitialization) {
    DataInfo info;
    info.param = "test_param";
    info.value = "test_value";
    info.type = DataInfo::Type::String;
    
    EXPECT_EQ(info.param, "test_param");
    EXPECT_EQ(info.value, "test_value");
    EXPECT_EQ(info.type, DataInfo::Type::String);
}

TEST(DataInfoTest, TypeEnumValues) {
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Zero), 0);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Int), 1);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Int64), 2);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::String), 3);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Double), 4);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Bool), 5);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::DateTime), 6);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::DateTimeNoSec), 7);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Date), 8);
    EXPECT_EQ(static_cast<int>(DataInfo::Type::Time), 9);
}

TEST(DataInfoTest, AllTypesAvailable) {
    // Ensure all types compile and are accessible
    DataInfo info;
    info.type = DataInfo::Type::Int;
    info.type = DataInfo::Type::Int64;
    info.type = DataInfo::Type::String;
    info.type = DataInfo::Type::Double;
    info.type = DataInfo::Type::Bool;
    info.type = DataInfo::Type::DateTime;
    info.type = DataInfo::Type::DateTimeNoSec;
    info.type = DataInfo::Type::Date;
    info.type = DataInfo::Type::Time;
    
    EXPECT_EQ(info.type, DataInfo::Type::Time);
}
