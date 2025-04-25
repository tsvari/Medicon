#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "include_util.h"

///////////////////////////////////////////////////////////
/// \brief The IConfigFile class
/// Use std::variant for different types
///  typedef std::variant<int, double, time_t, string> MyVariant
///
//https://en.cppreference.com/w/cpp/utility/variant/visit
// helper type for the visitor
template<class... Ts>
struct overloads : Ts... { using Ts::operator()...; };

/*
std::variant<int, float, std::string> var;
        var = 10;

        switch (var.index()) {
            case 0:
                std::cout << "Type is int: " << std::get<0>(var) << std::endl;
                break;
            case 1:
                std::cout << "Type is float: " << std::get<1>(var) << std::endl;
                break;
            case 2:
                std::cout << "Type is string: " << std::get<2>(var) << std::endl;
                break;
            default:
                std::cout << "Variant is valueless" << std::endl;
        }
if (std::holds_alternative<std::string>(var)) {
            std::cout << "Type is string: " << std::get<std::string>(var) << std::endl;
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

class ConfigFile
{
private:
    explicit ConfigFile(const  char *  allProjectPath, const  char *  projectName);

public:
    // Use in project
    static ConfigFile * Instance();
    // Use in tests
    static ConfigFile * InstanceCustom(const  char *  allProjectPath, const  char *  projectName);

    bool load();

    string value(const  char *  key);
    string & operator[](const  char *  key);

    string & xmlReadError() { return m_xmlReadError; }
    string appletPath () const {return m_appletePath;}
    string templatetPath () const {return m_templatePath;}
    string logFilePath () const {return m_logFilePath;}

    string projectPath () const;

protected:
    void setProjectPath(const  char *  m_allProjectPath, const  char *  projectName);

private:
    map<std::string, std::string> m_xmlData;

    string m_allProjectPath;
    string m_projectName;

    string m_configFilePath;
    string m_appletePath;
    string m_templatePath;
    string m_logFilePath;
    string m_xmlReadError;
};


#endif // CONFIGFILE_H
