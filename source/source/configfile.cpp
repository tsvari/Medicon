#include "configfile.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::string;
using std::map;

// ============================================================================
// ConfigFileBase Implementation
// ============================================================================

ConfigFileBase::ConfigFileBase(const char* allProjectPath, const char* projectName)
{
    setProjectPath(allProjectPath, projectName);
}

void ConfigFileBase::setProjectPath(const char* allProjectPath, const char* projectName)
{
    // Validate inputs
    if (!allProjectPath || std::strlen(allProjectPath) == 0) {
        throw std::invalid_argument(CONFIG_ERR_ALL_PROJECT_PATH);
    }
    
    if (!projectName || std::strlen(projectName) == 0) {
        throw std::invalid_argument(CONFIG_ERR_EMPTY_PROJECT_NAME);
    }

    m_allProjectPath = allProjectPath;
    m_projectName = projectName;
    
    // Ensure path ends with '/'
    if (m_allProjectPath.back() != '/') {
        m_allProjectPath.push_back('/');
    }

    // Validate project root directory exists
    if (!fs::is_directory(m_allProjectPath)) {
        std::ostringstream err;
        err << CONFIG_ERR_ALL_PROJECT_PATH << " Path: " << m_allProjectPath;
        throw std::invalid_argument(err.str());
    }

    // Construct paths using string streams for clarity
    std::ostringstream projectBase;
    projectBase << m_allProjectPath << m_projectName << "/";
    string projectBasePath = projectBase.str();

    m_configFilePath = projectBasePath + m_projectName + ".json";
    m_logFilePath = projectBasePath + "log/" + m_projectName + ".log";
    m_appletePath = projectBasePath + "sql-applets/";
    m_templatePath = projectBasePath + "templates/";

    // Validate config file exists
    if (!fs::exists(m_configFilePath)) {
        std::ostringstream err;
        err << CONFIG_ERR_CONFIG_FILE << " Path: " << m_configFilePath;
        throw std::invalid_argument(err.str());
    }
    
    // Validate log file exists
    if (!fs::exists(m_logFilePath)) {
        std::ostringstream err;
        err << CONFIG_ERR_LOG_FILE << " Path: " << m_logFilePath;
        throw std::invalid_argument(err.str());
    }
}

std::string& ConfigFileBase::operator[](const char* key)
{
    if (!m_jsonData.contains(key)) {
        std::ostringstream err;
        err << CONFIG_ERR_KEY_NOT_FOUND << ": " << key;
        throw std::out_of_range(err.str());
    }
    return m_jsonData[key];
}

string ConfigFileBase::value(const char* key) const
{
    if (!m_jsonData.contains(key)) {
        std::ostringstream err;
        err << CONFIG_ERR_KEY_NOT_FOUND << ": " << key;
        throw std::out_of_range(err.str());
    }
    return m_jsonData.at(key);
}

bool ConfigFileBase::contains(const char* key) const
{
    return m_jsonData.contains(key);
}

void ConfigFileBase::load()
{
    std::ifstream file(m_configFilePath);
    if (!file.is_open()) {
        std::ostringstream err;
        err << "Failed to open config file: " << m_configFilePath;
        throw std::runtime_error(err.str());
    }

    json doc;
    try {
        doc = json::parse(file);
    } catch (const json::parse_error& ex) {
        std::ostringstream err;
        err << "Invalid JSON in config file: " << m_configFilePath
            << " Error: " << ex.what();
        throw std::runtime_error(err.str());
    }

    try {
        m_jsonData = doc.get<map<string, string>>();
    } catch (const json::type_error& ex) {
        std::ostringstream err;
        err << "Config file must contain string key-value pairs: "
            << m_configFilePath << " Error: " << ex.what();
        throw std::runtime_error(err.str());
    }
}

string ConfigFileBase::projectPath() const
{
    return m_allProjectPath + m_projectName + "/";
}

// ============================================================================
// ConfigFile Singleton Implementation
// ============================================================================

ConfigFile::ConfigFile(const char* allProjectPath, const char* projectName)
    : ConfigFileBase(allProjectPath, projectName)
{
}

ConfigFile* ConfigFile::Instance()
{
    return InstanceCustom(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
}

ConfigFile* ConfigFile::InstanceCustom(const char* allProjectPath, const char* projectName)
{
    static ConfigFile configFile(allProjectPath, projectName);
    return &configFile;
}



