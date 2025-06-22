#include "include_backend_util.h"
#include "gtest/gtest.h"
#include <fstream>

namespace {
const std::string logoFile = std::string(ALL_BACKEND_TEST_APPDATA_PATH) + "logo.png";
}

TEST(SaBinaryTests, LoadFileTest)
{
    try {
        SaBinary::toStdString("C:/Path/To/Wrong/File");
    } catch(std::system_error & e) {
        // and this tests that it has the correct message
        EXPECT_TRUE(std::string(e.what()).contains(ERROR_BINARY_OPEN));
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }

    std::string stdString;
    EXPECT_NO_THROW(stdString = SaBinary::toStdString(logoFile.c_str()));

    SAString saString;
    EXPECT_NO_THROW(saString = SaBinary::toSaString(logoFile.c_str()));

    EXPECT_EQ(stdString, SaBinary::toStdString(saString));
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

int main() {
    // Example with a dummy stream (in a real scenario, this would be a file or network stream)
    std::ifstream fileStream("image.png", std::ios::binary);
    if (fileStream.is_open()) {
        std::string type = getImageTypeFromStream(fileStream);
        std::cout << "Image type: " << type << std::endl;
        fileStream.close();
    } else {
        std::cerr << "Error opening file." << std::endl;
    }
    return 0;
}
*/
