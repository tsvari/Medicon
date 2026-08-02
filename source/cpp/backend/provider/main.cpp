#include "../grpc/company/company_server.h"
#include "include_backend_util.h"
#include "configfile.h"

#include <easylogging++.h>
#include <filesystem>
#include <iostream>
#include <string>

INITIALIZE_EASYLOGGINGPP

/**
 * @brief Validate that a config key exists and return its value, or exit on failure
 */
static std::string requireConfig(ConfigFile& config, const char* key)
{
    if (!config.contains(key)) {
        std::cerr << "FATAL: Missing required config key '" << key << "' in provider.json"
                  << std::endl;
        std::exit(1);
    }
    return config.value(key);
}

int main()
{
    // ========================================================================
    // Phase 1: Load and validate configuration
    // ========================================================================

    ConfigFile config(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
    try {
        config.load();
    } catch (const std::exception& x) {
        std::cerr << "FATAL: Failed to load configuration: " << x.what() << std::endl;
        return 1;
    }

    // Validate that all required config keys are present
    // (catches misconfigured provider.json before the server starts)
    const std::string dbHost   = requireConfig(config, "host");
    const std::string dbUser   = requireConfig(config, "user");
    const std::string dbPass   = requireConfig(config, "pass");
    const bool logSql          = config.boolValueOr("log_sql", false);

    // Validate resource paths exist
    if (!std::filesystem::is_directory(config.appletPath())) {
        std::cerr << "FATAL: Applet directory not found: " << config.appletPath() << std::endl;
        return 1;
    }

    // ========================================================================
    // Phase 2: Initialize logger
    // ========================================================================

    el::Configurations qGlobalLog;
    qGlobalLog.setGlobally(el::ConfigurationType::Format, "%user:%fbase:%line:%datetime:%level:%msg:");
    qGlobalLog.setGlobally(el::ConfigurationType::Filename, config.logFilePath());
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToFile, "true");
    qGlobalLog.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "false");
    el::Loggers::setDefaultConfigurations(qGlobalLog, true);

    // Read server port from config (default 12345)
    uint16_t port = 12345;
    try {
        port = static_cast<uint16_t>(
            std::stoi(config.valueOr("port", "12345")));
    } catch (const std::exception&) {
        std::cerr << "FATAL: Invalid 'port' value in config" << std::endl;
        return 1;
    }

    // ========================================================================
    // Phase 3: Start gRPC server
    // ========================================================================

    RunCompanyServer(port, logSql,
                     config.appletPath(),
                     dbHost, dbUser, dbPass);

    return 0;
}
