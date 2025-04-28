#include "sqlconnection.h"
#include "gtest/gtest.h"

TEST(SqlConnectionTests, ConnectToDatabase)
{
    // Create object without pre initialization
    try {
        SqlConnection connection;
    } catch(const std::runtime_error & e) {
        // and this tests that it has the correct message
        EXPECT_STREQ(SQL_CONNECTION_ERR_INIT, e.what());
    } catch (...) {
        FAIL() << "Expected a different exception type.";
    }
}
