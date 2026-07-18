#include "configfile.h"
#include "gtest/gtest.h"

#include "sqlcommand.h"
#include "sqlconnection.h"
#include "sqlquery.h"
#include "sqltemplate.h"
#include "JsonParameterFormatter.h"

TEST(ConnectionIntegrationTests, LoadAndCheckData)
{
    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    EXPECT_NO_THROW(config.load());

    // SqlTemplate takes full paths; no setSearchPath needed

    // Inilialize sql connections with data: host, user, pass
    SqlConnection::InitAllConnections(SA_PostgreSQL_Client,
                                      config.value("host").c_str(),
                                      config.value("user").c_str(),
                                      config.value("pass").c_str());

    SqlConnection con;
    try {
        con.connect();
    } catch(SAException & x) {
        FAIL() << x.ErrText().GetMultiByteChars();
        try {
            con.rollback();
        } catch(SAException &) {
        }
    } catch(const SqlTemplateException & e) {
        FAIL() << e.what();
    } catch(...) {
        FAIL() << "Unknown error";
    }

    SUCCEED() << "Connection succedeed!";

}





