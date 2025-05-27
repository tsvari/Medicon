#pragma once

#include <vector>
#include <string>
#include <map>
#include <variant>
#include <set>

using std::string;
using std::vector;
using std::map;
using std::variant;
using std::set;

struct DataInfo {
    enum Type{
        Int = 0,
        Int64,
        String,
        Double,
        DateTime,
        DateTimeNoSec,
        Date,
        Time
    };
    string param;
    string value;
    Type type;
};

namespace Trimmer {
    extern const char * ws;
    // trim from end of string (right)
    std::string & rtrim(std::string & s, const char* t = ws);
    // trim from beginning of string (left)
    std::string & ltrim(std::string & s, const char* t = ws);
    // trim from both ends of string (right then left)
    std::string & trim(std::string & s, const char* t = ws);
};

