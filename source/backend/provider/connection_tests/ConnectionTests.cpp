#include "configfile.h"
#include "gtest/gtest.h"

#include "sqlapplet.h"
#include "sqlcommand.h"
#include "sqlconnection.h"
#include "sqlquery.h"
#include "JsonParameterFormatter.h"

TEST(ConnectionIntegrationTests, LoadAndCheckData)
{
    ConfigFile * config = nullptr;
    EXPECT_NO_THROW(config = ConfigFile::Instance());
    EXPECT_TRUE(config->load());

    SQLApplet::InitPathToApplets(config->appletPath().c_str());

    // Inilialize sql connections with data: host, user, pass
    SqlConnection::InitAllConnections(SA_PostgreSQL_Client,
                                      config->value("host").c_str(),
                                      config->value("user").c_str(),
                                      config->value("pass").c_str());

    SqlConnection con;
    try {
        con.connect();
    } catch(SAException & x) {
        FAIL() << x.ErrText().GetMultiByteChars();
        try {
            con.rollback();
        } catch(SAException &) {
        }
    } catch(const SQLAppletException & e) {
        FAIL() << e.what();
    } catch(...) {
        FAIL() << "Unknown error";
    }

    SUCCEED() << "Connection succedeed!";

}





