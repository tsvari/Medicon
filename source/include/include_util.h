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

namespace {
    string trimLeftRight(const string & str) {
        size_t first = str.find_first_not_of(' ');
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last-first + 1));
    }
}

//extern el::Configurations qGlobalLog;
