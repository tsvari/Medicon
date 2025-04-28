#include <iostream>

#include <SQLAPI.h>
#include <easylogging++.h>

#include "include_backend_util.h"
#include "configfile.h"
#include "sqlapplet.h"

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

    // SQLAPI example
    SAString  db_host = config->value("host").c_str();
    SAString  db_user = config->value("user").c_str();
    SAString  db_pass = config->value("pass").c_str();
    SAConnection con;
    try
    {
        con.setClient( SA_PostgreSQL_Client );
        con.Connect(db_host, db_user, db_pass);
        SACommand select(&con, _TSA(""));

        con.Disconnect();

    } catch(SAException & x) {
        // SAConnection::Rollback()
        // can also throw an exception
        // (if a network error for example),
        // we will be ready
        try
        {
            // on error rollback changes
            con.Rollback();
        } catch(SAException &) {

        }
        // print error message
        //LOG(ERROR) << "DBConnection: " << x.ErrText().GetMultiByteChars();
    }

    return 0;
}
