#include "../include_util.h"
#include "gtest/gtest.h"

using BackendUtil::sqlRowOffset;

TEST(UtilFunctionsTests, BackendUtilTest)
{
    uint32_t page = 1;
    uint32_t limitCount = 30;
    uint32_t totalCount = 100;

    uint32_t  realCurrentPage = 0; // returns throw parameter
    uint32_t pageCount = 0; // returns throw parameter

    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);

    page = 2;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 30);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);

    page = 3;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 60);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);

    page = 4;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 90);
    EXPECT_EQ(realCurrentPage, page);
    EXPECT_EQ(pageCount, 4);

    page = 5;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 4);

    page = 2;
    limitCount = 100;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 0);

    page = 20;
    limitCount = 300;
    EXPECT_EQ(sqlRowOffset(page, limitCount, totalCount, realCurrentPage, pageCount), 0);
    EXPECT_EQ(realCurrentPage, 0);
    EXPECT_EQ(pageCount, 0);
}


