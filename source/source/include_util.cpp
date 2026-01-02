#include "include_util.h"
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <system_error>

namespace {
constexpr const char* ERROR_BINARY_OPEN = "Failed to open file!";
}

using std::string;
using std::string_view;
// ============================================================================
// String Trimming Utilities
// ============================================================================

namespace Trimmer {
string rtrim(string_view s, const char* t) {
    string result(s);
    return rtrimInPlace(result, t);
}

string ltrim(string_view s, const char* t) {
    string result(s);
    return ltrimInPlace(result, t);
}

string trim(string_view s, const char* t) {
    string result(s);
    return trimInPlace(result, t);
}

string& rtrimInPlace(string& s, const char* t) {
    if (s.empty()) {
        return s;
    }
    auto pos = s.find_last_not_of(t);
    if (pos != string::npos) {
        s.erase(pos + 1);
    } else {
        s.clear();  // String contains only trim characters
    }
    return s;
}

string& ltrimInPlace(string& s, const char* t) {
    if (s.empty()) {
        return s;
    }
    auto pos = s.find_first_not_of(t);
    if (pos != string::npos) {
        s.erase(0, pos);
    } else {
        s.clear();  // String contains only trim characters
    }
    return s;
}

string& trimInPlace(string& s, const char* t) {
    return ltrimInPlace(rtrimInPlace(s, t), t);
}
}

// ============================================================================
// Common Utilities - now inline in header
// ============================================================================

// ============================================================================
// Binary File Utilities
// ============================================================================

namespace StdBinary {
string toStdString(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::system_error(
            errno,
            std::generic_category(),
            string(ERROR_BINARY_OPEN) + " Path: " + path.string()
            );
    }

    // Read entire file efficiently
    // Use extra parentheses to avoid Most Vexing Parse
    string content(
        (std::istreambuf_iterator<char>(file)),
        (std::istreambuf_iterator<char>())
        );

    return content;
    // File automatically closed by RAII
}

void toBinary(const std::filesystem::path& path, string_view data) {
    std::ofstream file(path, std::ios::binary | std::ios::out);
    if (!file.is_open()) {
        throw std::system_error(
            errno,
            std::generic_category(),
            string(ERROR_BINARY_OPEN) + " Path: " + path.string()
            );
    }

    file.write(data.data(), data.size());

    // Check for write errors
    if (!file.good()) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "Failed to write data to file: " + path.string()
            );
    }

    // File automatically closed by RAII
}
}
