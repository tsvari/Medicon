#include <iostream>

#include <easylogging++.h>

#include "include_backend_util.h"
#include "configfile.h"
#include "sqlapplet.h"
#include "sqlconnection.h"

INITIALIZE_EASYLOGGINGPP

int main()
{

    ConfigFile * config = ConfigFile::Instance();
    if(!config->load()) {
        LOG(INFO) << config->xmlReadError();
        return -1;
    }

    // Initialize logger with global settings
    el::Configurations qGlobalLog;
    qGlobalLog.setGlobally(el::ConfigurationType::Format, "%user:%fbase:%line:%datetime:%level:%msg:");
    qGlobalLog.setGlobally(el::ConfigurationType::Filename, config->logFilePath());
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToFile, "true");
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::setDefaultConfigurations(qGlobalLog, true);

    // Initialize project applet path
    SQLApplet::InitPathToApplets(config->appletPath().c_str());

    // Inilialize sql connections with data: host, user, pass
    SqlConnection::InitAllConnections(SA_PostgreSQL_Client,
                                        config->value("host").c_str(),
                                        config->value("user").c_str(),
                                        config->value("pass").c_str());

    SqlConnection con;
    try
    {
        con.connect();
        SACommand select(con.connectionSa(), _TSA(""));

        //con.Disconnect();

    } catch(SAException & x) {
        // SAConnection::Rollback()
        // can also throw an exception
        // (if a network error for example),
        // we will be ready
        try
        {
            // on error rollback changes
            con.rollback();
        } catch(SAException &) {

        }
        // print error message
        //LOG(ERROR) << "DBConnection: " << x.ErrText().GetMultiByteChars();
    }

    return 0;
}
