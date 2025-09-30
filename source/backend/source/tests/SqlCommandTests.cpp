#include "sqlcommand.h"
#include "gtest/gtest.h"

#include "sqlconnection.h"

// postgresql last inserted row id
// https://stackoverflow.com/questions/2944297/postgresql-function-for-last-inserted-id
// INSERT INTO persons (lastname,firstname) VALUES ('Smith', 'John') RETURNING id;
// update customer set "NICNAME" = 'Givi' where "UID" = '18f23eaf-286b-4939-875c-8d2c5d8ec8d9' RETURNING "UID"
// https://www.sqlapi.com/HowTo/fetch/
// bool is_result = cmd.isResultSet();

TEST(SqlCommandTests, VoidTest)
{
    std::string input = "2007-01-20 10:11:12";
    std::chrono::milliseconds sysSecs = TimeFormatHelper::stringTochronoSysSec(input, DataInfo::DateTime);

    SQLApplet::InitPathToApplets(ALL_BACKEND_TEST_APPDATA_PATH);
    SqlConnection::InitAllConnections(SA_PostgreSQL_Client, "host", "user", "pass");

    SqlConnection con;
    SqlCommand command(con, "test.xml");
    command.addDataInfo("Money", 122.123);
    command.addDataInfo("Height", 175);
    command.addDataInfo("BirthTime", sysSecs, DataInfo::Time);
    command.addDataInfo("WholeDateTime", sysSecs, DataInfo::DateTime);
    command.addDataInfo("BirthDate", sysSecs, DataInfo::Date);
    command.addDataInfo("Name", "Givi");
    // No connection
    EXPECT_THROW(command.execute(), SAException);

    string actual = "Money=122.123000,Height=175,BirthTime='10:11:12',WholeDateTime='2007-01-20 10:11:12',BirthDate='2007-01-20',Name='Givi'";
    string expected = command.CommandText().GetMultiByteChars();

    EXPECT_TRUE(expected.find(actual) != std::string::npos);
}
