#include "configfile.h"
#include "Markup.h"
#include <filesystem>

namespace fs = std::filesystem;

ConfigFile::ConfigFile(const char * allProjectPath, const  char *  projectName)
{
    setProjectPath(allProjectPath, projectName);
}

ConfigFile * ConfigFile::Instance()
{
    return InstanceCustom(ALL_PROJECT_APPDATA_PATH, PROJECT_NAME);
}

ConfigFile * ConfigFile::InstanceCustom(const  char *  allProjectPath, const  char *  projectName)
{
    static ConfigFile configFile(allProjectPath, projectName);
    return & configFile;
}

void ConfigFile::setProjectPath(const  char *  allProjectPath, const  char *  projectName)
{
    m_allProjectPath = allProjectPath;
    m_projectName = projectName;
    if(m_allProjectPath.size() > 0 && m_allProjectPath.back() != '/') {
        m_allProjectPath.push_back('/');
    }
    //fs::path absolutePath = fs::absolute(projectPath);
    if(!fs::is_directory(m_allProjectPath)) {
        throw std::invalid_argument(CONFIG_ERR_ALL_PROJECT_PATH);
    }

    m_configFilePath = m_allProjectPath + string(m_projectName) + string("/") + string(m_projectName) + string(".xml");
    m_logFilePath = m_allProjectPath + string(m_projectName) + string("/log/") + string(m_projectName) + string(".log");

    if(!fs::exists(m_configFilePath)) {
        throw std::invalid_argument(CONFIG_ERR_CONFIG_FILE);
    }
    if(!fs::exists(m_logFilePath)) {
        throw std::invalid_argument(CONFIG_ERR_LOG_FILE);
    }

    m_appletePath = m_allProjectPath + string(m_projectName) + string("/sql-applets/");
    m_templatePath = m_allProjectPath + string(m_projectName) + string("/templates/");
}

std::string & ConfigFile::operator[](const  char *  key)
{
    if(!m_xmlData.contains(key)) {
        throw std::out_of_range("Index out of bounds");
    }
    return m_xmlData[key];
}

string ConfigFile::value(const char *key)
{
    if(!m_xmlData.contains(key)) {
        throw std::out_of_range("Index out of bounds");
    }
    return m_xmlData[key];
}

//////////////////////////////////////////////////////////////////////////////
/// \brief ConfigFile::load
/// \return
///
bool ConfigFile::load()
{
    CMarkup xmlFile;
    if (!xmlFile.Load(m_configFilePath)) {
        m_xmlReadError = "Config file: " + xmlFile.GetError();
        return false;
    }

    xmlFile.IntoElem();

    while(xmlFile.FindChildElem()) {
        m_xmlData[xmlFile.GetChildTagName()] = xmlFile.GetChildData();
    }

    return true;
}

string ConfigFile::projectPath () const
{
    return m_allProjectPath + "/" + string(m_projectName);
}



