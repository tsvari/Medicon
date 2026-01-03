#include "include_backend_util.h"
#include "gtest/gtest.h"
#include <fstream>
#include <filesystem>

namespace {
const std::string logoFile = std::string(ALL_BACKEND_TEST_APPDATA_PATH) + "logo.png";
const std::string testDataPath = std::string(ALL_BACKEND_TEST_APPDATA_PATH);
}

/*
JPEG: FF D8 FF E0 (or E1, E2, etc.)
       PNG: 89 50 4E 47 0D 0A 1A 0A
       GIF: 47 49 46 38 37 61 or 47 49 46 38 39 61
       BMP: 42 4D

std::string getImageTypeFromStream(std::istream& is) {
    std::vector<char> header(8); // Read the first 8 bytes
    is.read(header.data(), header.size());

    // Compare with known magic numbers
    if (header[0] == (char)0xFF && header[1] == (char)0xD8 && header[2] == (char)0xFF) {
        return "JPEG";
    } else if (header[0] == (char)0x89 && header[1] == (char)0x50 && header[2] == (char)0x4E && header[3] == (char)0x47) {
        return "PNG";
    } else if (header[0] == (char)0x47 && header[1] == (char)0x49 && header[2] == (char)0x46) {
        return "GIF";
    } else if (header[0] == (char)0x42 && header[1] == (char)0x4D) {
        return "BMP";
    }
    return "Unknown";
}
*/
// ============================================================================
// File Loading Tests
// ============================================================================

TEST(SaBinaryTests, LoadInvalidFile) {
    EXPECT_THROW(
        SaBinary::toStdString("C:/Path/To/NonExistent/File.bin"),
        std::system_error
    );
}

TEST(SaBinaryTests, LoadFileToStdString) {
    if (!std::filesystem::exists(logoFile)) {
        GTEST_SKIP() << "Test file not found: " << logoFile;
    }

    std::string stdString;
    EXPECT_NO_THROW(stdString = SaBinary::toStdString(logoFile.c_str()));
    EXPECT_FALSE(stdString.empty());
    
    // PNG files start with specific magic bytes
    EXPECT_EQ(static_cast<unsigned char>(stdString[0]), 0x89);
    EXPECT_EQ(stdString[1], 'P');
    EXPECT_EQ(stdString[2], 'N');
    EXPECT_EQ(stdString[3], 'G');
}

TEST(SaBinaryTests, LoadFileToSaString) {
    if (!std::filesystem::exists(logoFile)) {
        GTEST_SKIP() << "Test file not found: " << logoFile;
    }

    SAString saString;
    EXPECT_NO_THROW(saString = SaBinary::toSaString(logoFile.c_str()));
    EXPECT_GT(saString.GetBinaryLength(), 0);
}

// ============================================================================
// Conversion Tests
// ============================================================================

TEST(SaBinaryTests, ConvertSaStringToStdString) {
    if (!std::filesystem::exists(logoFile)) {
        GTEST_SKIP() << "Test file not found: " << logoFile;
    }

    SAString saString = SaBinary::toSaString(logoFile.c_str());
    std::string stdString = SaBinary::toStdString(saString);
    
    EXPECT_FALSE(stdString.empty());
    EXPECT_EQ(stdString.size(), saString.GetBinaryLength());
}

TEST(SaBinaryTests, ConvertStdStringToSaString) {
    std::string original = "Binary data \x00\x01\x02\xFF test";
    
    SAString saString = SaBinary::toSaString(original);
    
    EXPECT_EQ(saString.GetBinaryLength(), original.size());
    
    // Convert back and verify
    std::string converted = SaBinary::toStdString(saString);
    EXPECT_EQ(converted, original);
}

TEST(SaBinaryTests, RoundTripConversion) {
    if (!std::filesystem::exists(logoFile)) {
        GTEST_SKIP() << "Test file not found: " << logoFile;
    }

    // Load as std::string
    std::string stdString1 = SaBinary::toStdString(logoFile.c_str());
    
    // Convert to SAString
    SAString saString = SaBinary::toSaString(stdString1);
    
    // Convert back to std::string
    std::string stdString2 = SaBinary::toStdString(saString);
    
    // Should be identical
    EXPECT_EQ(stdString1, stdString2);
    EXPECT_EQ(stdString1.size(), stdString2.size());
}

TEST(SaBinaryTests, FilePathConversionEquality) {
    if (!std::filesystem::exists(logoFile)) {
        GTEST_SKIP() << "Test file not found: " << logoFile;
    }

    // Load directly to std::string
    std::string stdString = SaBinary::toStdString(logoFile.c_str());
    
    // Load directly to SAString then convert
    SAString saString = SaBinary::toSaString(logoFile.c_str());
    std::string fromSaString = SaBinary::toStdString(saString);
    
    // Both methods should produce identical results
    EXPECT_EQ(stdString, fromSaString);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(SaBinaryTests, EmptyStdString) {
    std::string empty;
    SAString saString = SaBinary::toSaString(empty);
    
    EXPECT_EQ(saString.GetBinaryLength(), 0);
    
    std::string converted = SaBinary::toStdString(saString);
    EXPECT_TRUE(converted.empty());
}

TEST(SaBinaryTests, BinaryDataWithNullBytes) {
    // String with embedded null bytes
    std::string original;
    original.push_back('A');
    original.push_back('\0');
    original.push_back('B');
    original.push_back('\0');
    original.push_back('C');
    
    SAString saString = SaBinary::toSaString(original);
    std::string converted = SaBinary::toStdString(saString);
    
    EXPECT_EQ(converted.size(), 5);
    EXPECT_EQ(converted, original);
    EXPECT_EQ(converted[0], 'A');
    EXPECT_EQ(converted[1], '\0');
    EXPECT_EQ(converted[2], 'B');
}

TEST(SaBinaryTests, LargeBinaryData) {
    // Create a large binary string (1 MB)
    std::string large(1024 * 1024, 'X');
    
    SAString saString = SaBinary::toSaString(large);
    EXPECT_EQ(saString.GetBinaryLength(), large.size());
    
    std::string converted = SaBinary::toStdString(saString);
    EXPECT_EQ(converted.size(), large.size());
    EXPECT_EQ(converted, large);
}
