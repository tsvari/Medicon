#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "include_util.h"
#include <map>
#include <string>

namespace {
const char * CONFIG_ERR_ALL_PROJECT_PATH = "Config: All project path not found!";
const char * CONFIG_ERR_CONFIG_FILE = "Config: Config file was not found";
const char * CONFIG_ERR_LOG_FILE = "Config: Log file was not found";
const char * CONFIG_ERR_EMPTY_PROJECT_NAME = "Config: Project name cannot be empty";
const char * CONFIG_ERR_KEY_NOT_FOUND = "Config: Key not found in configuration";
}

/**
 * @class ConfigFileBase
 * @brief Base class for configuration file management
 * 
 * Provides all configuration functionality without singleton restrictions.
 * Inherit from this class for testable implementations.
 */
class ConfigFileBase
{
protected:
    /**
     * @brief Protected constructor for base class
     * @param allProjectPath Root path containing all projects
     * @param projectName Name of the specific project
     * @throws std::invalid_argument if paths are invalid or files don't exist
     */
    explicit ConfigFileBase(const char* allProjectPath, const char* projectName);
    
    /**
     * @brief Default constructor for derived classes
     */
    ConfigFileBase() = default;

public:
    virtual ~ConfigFileBase() = default;

    /**
     * @brief Load configuration from JSON file
     * @throws std::runtime_error if file cannot be opened or JSON is invalid
     * 
     * Must be called before accessing configuration values.
     */
    virtual void load();

    /**
     * @brief Get configuration value by key
     * @param key Configuration key
     * @return Configuration value as string
     * @throws std::out_of_range if key doesn't exist
     */
    virtual std::string value(const char* key) const;
    
    /**
     * @brief Get mutable reference to configuration value
     * @param key Configuration key
     * @return Reference to configuration value
     * @throws std::out_of_range if key doesn't exist
     * 
     * Allows modification: config["key"] = "new_value"
     */
    virtual std::string& operator[](const char* key);

    /**
     * @brief Check if configuration key exists
     * @param key Configuration key
     * @return true if key exists, false otherwise
     */
    virtual bool contains(const char* key) const;

    // Path getters
    std::string appletPath() const { return m_appletePath; }
    std::string templatePath() const { return m_templatePath; }
    std::string logFilePath() const { return m_logFilePath; }
    std::string configFilePath() const { return m_configFilePath; }
    std::string projectPath() const;

protected:
    /**
     * @brief Initialize and validate project paths
     * @param allProjectPath Root path containing all projects
     * @param projectName Name of the specific project
     * @throws std::invalid_argument if paths are invalid or files don't exist
     */
    void setProjectPath(const char* allProjectPath, const char* projectName);

    std::map<std::string, std::string> m_jsonData;

    std::string m_allProjectPath;
    std::string m_projectName;

    std::string m_configFilePath;
    std::string m_appletePath;
    std::string m_templatePath;
    std::string m_logFilePath;
};

/**
 * @class ConfigFile
 * @brief Singleton class for managing project configuration files
 * 
 * Provides thread-safe access to project configuration stored in JSON format.
 * Manages paths to project resources (config, logs, applets, templates).
 * 
 * Usage:
 * @code
 * ConfigFile* config = ConfigFile::Instance();
 * config->load();
 * std::string dbHost = config->value("db_host");
 * @endcode
 * 
 * For testing, use ConfigFileForTesting instead of this singleton.
 */
class ConfigFile : public ConfigFileBase
{
private:
    /**
     * @brief Private constructor for singleton pattern
     * @param allProjectPath Root path containing all projects
     * @param projectName Name of the specific project
     */
    explicit ConfigFile(const char* allProjectPath, const char* projectName);

public:
    /**
     * @brief Get singleton instance using default project paths
     * @return Pointer to ConfigFile singleton
     * 
     * Uses ALL_PROJECT_APPDATA_PATH and PROJECT_NAME macros.
     */
    static ConfigFile* Instance();
    
    /**
     * @brief Get singleton instance with custom paths (for testing)
     * @param allProjectPath Root path containing all projects
     * @param projectName Name of the specific project
     * @return Pointer to ConfigFile singleton
     * 
     * NOTE: For tests, prefer using ConfigFileForTesting which doesn't use singleton pattern.
     */
    static ConfigFile* InstanceCustom(const char* allProjectPath, const char* projectName);

    // Delete copy and move constructors/operators (singleton pattern)
    ConfigFile(const ConfigFile&) = delete;
    ConfigFile& operator=(const ConfigFile&) = delete;
    ConfigFile(ConfigFile&&) = delete;
    ConfigFile& operator=(ConfigFile&&) = delete;
};

/**
 * @class ConfigFileForTesting
 * @brief Testable configuration class without singleton restrictions
 * 
 * Use this class in tests instead of ConfigFile singleton.
 * Each instance is independent, allowing proper test isolation.
 * 
 * Usage in tests:
 * @code
 * ConfigFileForTesting config("/test/path", "TestProject");
 * config.load();
 * EXPECT_EQ(config.value("key"), "value");
 * @endcode
 */
class ConfigFileForTesting : public ConfigFileBase
{
public:
    /**
     * @brief Public constructor for testing
     * @param allProjectPath Root path containing all projects
     * @param projectName Name of the specific project
     * @throws std::invalid_argument if paths are invalid or files don't exist
     */
    explicit ConfigFileForTesting(const char* allProjectPath, const char* projectName)
        : ConfigFileBase(allProjectPath, projectName)
    {
    }
};

#endif // CONFIGFILE_H
