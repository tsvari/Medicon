#include "configfile.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
using std::string;
using std::map;

namespace {
string toLowerCopy(string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

bool parseBool(string raw, bool& out)
{
    raw = toLowerCopy(raw);

    if (raw == "true" || raw == "1" || raw == "yes" || raw == "on") {
        out = true;
        return true;
    }
    if (raw == "false" || raw == "0" || raw == "no" || raw == "off") {
        out = false;
        return true;
    }
    return false;
}
}

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

string ConfigFileBase::valueOr(const char* key, string defaultValue) const
{
    if (!key) {
        return defaultValue;
    }
    auto it = m_jsonData.find(key);
    if (it == m_jsonData.end()) {
        return defaultValue;
    }
    return it->second;
}

bool ConfigFileBase::boolValueOr(const char* key, bool defaultValue) const
{
    if (!key) {
        return defaultValue;
    }

    auto it = m_jsonData.find(key);
    if (it == m_jsonData.end()) {
        return defaultValue;
    }

    bool parsed = false;
    if (!parseBool(it->second, parsed)) {
        std::ostringstream err;
        err << "Config: Key '" << key << "' is not a valid boolean value: '" << it->second << "'";
        throw std::runtime_error(err.str());
    }
    return parsed;
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

    if (!doc.is_object()) {
        std::ostringstream err;
        err << "Config file must contain a JSON object at root: " << m_configFilePath;
        throw std::runtime_error(err.str());
    }

    m_jsonData.clear();
    for (auto& [key, value] : doc.items()) {
        if (value.is_string()) {
            m_jsonData[key] = value.get<string>();
        } else if (value.is_number_integer()) {
            m_jsonData[key] = std::to_string(value.get<int64_t>());
        } else if (value.is_number_float()) {
            m_jsonData[key] = std::to_string(value.get<double>());
        } else if (value.is_boolean()) {
            m_jsonData[key] = value.get<bool>() ? "true" : "false";
        } else if (value.is_null()) {
            m_jsonData[key] = "NULL";
        } else {
            // Arrays/objects: serialize to string
            m_jsonData[key] = value.dump();
        }
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



