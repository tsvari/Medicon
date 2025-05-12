#include "../grpc/company_server.hpp"
#include "include_backend_util.h"
#include "configfile.h"
#include "sqlapplet.h"
#include "sqlconnection.h"

#include <easylogging++.h>
INITIALIZE_EASYLOGGINGPP

int main()
{

    // Initialize and open config file
    ConfigFile * config = ConfigFile::Instance();
    if(!config->load()) {
        throw config->xmlReadError();
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

    RunCompanyServer(12345);

    return 0;
}
