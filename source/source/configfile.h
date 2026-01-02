#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "include_util.h"
#include <map>

///////////////////////////////////////////////////////////
/// \brief The IConfigFile class
/// Use std::variant for different types
///  typedef std::variant<int, double, time_t, std::string> MyVariant
///
//https://en.cppreference.com/w/cpp/utility/variant/visit
// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

/*
std::variant<int, float, std::std::string> var;
        var = 10;

        switch (var.index()) {
            case 0:
                std::cout << "Type is int: " << std::get<0>(var) << std::endl;
                break;
            case 1:
                std::cout << "Type is float: " << std::get<1>(var) << std::endl;
                break;
            case 2:
                std::cout << "Type is std::string: " << std::get<2>(var) << std::endl;
                break;
            default:
                std::cout << "Variant is valueless" << std::endl;
        }
if (std::holds_alternative<std::std::string>(var)) {
            std::cout << "Type is std::string: " << std::get<std::std::string>(var) << std::endl;
        } else if (std::holds_alternative<int>(var)) {
             std::cout << "Type is int: " << std::get<int>(var) << std::endl;
        } else if (std::holds_alternative<float>(var)){
             std::cout << "Type is float: " << std::get<float>(var) << std::endl;
        }
std::variant<int, float> var = 10;

if (int* ptr = std::get_if<int>(&var)) {
  // var holds an int, ptr points to the int value
} else if (float* ptr = std::get_if<float>(&var)) {
  // var holds a float, ptr points to the float value
} else {
  // var is in an invalid state
}

to use std::visit
search google std::visit example c++
*/

namespace {
const char * CONFIG_ERR_ALL_PROJECT_PATH = "Config: All project path not found!";
const char * CONFIG_ERR_CONFIG_FILE = "Config: Config file was not found";
const char * CONFIG_ERR_LOG_FILE = "Config: Log file was not found";
}

class ConfigFile
{
private:
    explicit ConfigFile(const  char *  allProjectPath, const  char *  projectName);

public:
    // Use in project
    static ConfigFile * Instance();
    // Use in tests
    static ConfigFile * InstanceCustom(const  char *  allProjectPath, const  char *  projectName);

    void load();

    std::string value(const  char *  key);
    std::string & operator[](const  char *  key);

    std::string appletPath() const {return m_appletePath;}
    std::string templatetPath() const {return m_templatePath;}
    std::string logFilePath() const {return m_logFilePath;}

    std::string projectPath() const;

protected:
    void setProjectPath(const  char *  m_allProjectPath, const  char *  projectName);

private:
    std::map<std::string, std::string> m_jsonData;

    std::string m_allProjectPath;
    std::string m_projectName;

    std::string m_configFilePath;
    std::string m_appletePath;
    std::string m_templatePath;
    std::string m_logFilePath;
    std::string m_readError;
};


#endif // CONFIGFILE_H
