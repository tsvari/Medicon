#include "include_util.h"

#include <stdexcept>

namespace CommonUtil {
    uint32_t sqlRowOffset(uint32_t page, uint32_t limitCount, uint32_t totalCount,
                      uint32_t & realCurrentPage, uint32_t & pageCount) {
        if (limitCount == 0) {
            throw std::runtime_error("Division by zero!");
        }
        if(limitCount >= totalCount) {
            realCurrentPage = 0;
            pageCount = 0;
            return 0;
        }
        pageCount = totalCount / limitCount;
        if(totalCount % limitCount > 0) {
            pageCount++;
        }
        if(page > pageCount) {
            realCurrentPage = 0;
            return 0;
        }

        realCurrentPage = page;
        return limitCount * (page - 1);
    }

    uint32_t sqlRowOffset(uint32_t page, uint32_t limitCount, uint32_t totalCount) {
        uint32_t realCurrentPage = 0;
        uint32_t pageCount = 0;
        return sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount);
    }
};

namespace Trimmer {
    const char * ws = " \t\n\r\f\v";
    // trim from end of string (right)
    std::string & rtrim(std::string & s, const char * t)
    {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }
    // trim from beginning of string (left)
    std::string& ltrim(std::string & s, const char * t)
    {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }
    // trim from both ends of string (right then left)
    std::string& trim(std::string & str, const char * t)
    {
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
        return ltrim(rtrim(str, t), t);
    }
};


