#include "configfile.h"
#include "Markup.h"
#include <filesystem>
#include <cassert>

namespace fs = std::filesystem;

ConfigFile::ConfigFile(const char * allProjectPath, const  char *  projectName)
{
    setProjectPath(allProjectPath, projectName);
}

void ConfigFile::setProjectPath(const  char *  allProjectPath, const  char *  projectName)
{
    allProjectPath_ = allProjectPath;
    projectName_ = projectName;
    if(allProjectPath_.back() != '/') {
        allProjectPath_.push_back('/');
    }
    //fs::path absolutePath = fs::absolute(projectPath);
    assert(fs::is_directory(allProjectPath_));

    configFilePath_ = allProjectPath_ + string(projectName_) + string("/") + string(projectName_) + string(".xml");
    logFilePath_ = allProjectPath_ + string(projectName_) + string("/log/") + string(projectName_) + string(".log");
    assert(fs::exists(configFilePath_));
    assert(fs::exists(logFilePath_));

    appletePath_ = allProjectPath_ + string(projectName_) + string("/sql-applets/");
    templatePath_ = allProjectPath_ + string(projectName_) + string("/templates/");
}

std::string & ConfigFile::operator[](const  char *  key)
{
    if(!xmlData_.contains(key)) {
        throw std::out_of_range("Index out of bounds");
    }
    return xmlData_[key];
}

//////////////////////////////////////////////////////////////////////////////
/// \brief ConfigFile::load
/// \return
///
bool ConfigFile::load()
{
    CMarkup xmlFile;
    if (!xmlFile.Load(configFilePath_)) {
        lastErrorMsg_ = "Config file: " + xmlFile.GetError();
        return false;
    }

    xmlFile.IntoElem();

    while(xmlFile.FindChildElem()) {
        xmlData_[xmlFile.GetChildTagName()] = xmlFile.GetChildData();
    }

    return true;
}

string ConfigFile::projectPath () const
{
    return allProjectPath_ + "/" + string(projectName_);
}
//////////////////////////////////////////////////////////////////////////////
/// \brief ConfigFile::value
/// \param inParam
/// \param out_value
/// \return
///
bool ConfigFile::value(const char * inParam, std::string & valueOut) const
{
    std::map<std::string, std::string>::const_iterator it = xmlData_.find(inParam);
    if(it == xmlData_.end()) {
        return false;
    }
    valueOut = it->second;
    return true;
}


