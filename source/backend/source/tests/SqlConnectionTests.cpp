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

    //SqlConnection con;
    //try
    //{
    //    con.connect();
    //    SACommand select(con.connectionSa(), _TSA(""));

    //    //con.Disconnect();

    //} catch(SAException & x) {
    //    // SAConnection::Rollback()
    //    // can also throw an exception
    //    // (if a network error for example),
    //    // we will be ready
    //    try
    //    {
    //        // on error rollback changes
    //        con.rollback();
    //    } catch(SAException &) {

    //    }
    //    // print error message
    //    //LOG(ERROR) << "DBConnection: " << x.ErrText().GetMultiByteChars();
    //}

}
