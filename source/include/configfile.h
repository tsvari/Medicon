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

class IConfigFile
{
    virtual bool load() = 0;
};

class ConfigFile : public IConfigFile
{
public:
    explicit ConfigFile(const  char *  allProjectPath, const  char *  projectName);
    ConfigFile(){}

    void setProjectPath(const  char *  allProjectPath, const  char *  projectName);

    std::string & operator[](const  char *  key);

public:
    virtual bool load();
    bool value(const char * inParam, std::string & out_value) const;

    inline string &	error() { return lastErrorMsg_; }

    inline string appletPath () const {return appletePath_;}
    inline string templatetPath () const {return templatePath_;}
    inline string logFilePath () const {return logFilePath_;}

    string projectPath () const;

private:
    map<std::string, std::string> xmlData_;

    string allProjectPath_;
    string projectName_;

    string configFilePath_;
    string appletePath_;
    string templatePath_;
    string logFilePath_;
    string lastErrorMsg_;
};


#endif // CONFIGFILE_H
