#ifndef SQLAPPLET_H
#define SQLAPPLET_H

#include <stdexcept>
#include "include_util.h"
#include "TypeToStringFormatter.h"

class DataInfo;
class SQLAppletException: public std::runtime_error
{
public:
    explicit SQLAppletException( const char * msg ): std::runtime_error( msg ) {
        strcpy(em, msg);
    }

    char * GetError() { return em; }
private:
    char em[1024];

};

class SQLApplet
{
public:
    SQLApplet(const char * appletName, map<string, string> formattedParamValueList = {});
    SQLApplet(const char * appletName, TypeToStringFormatter const & formatter);
    ~SQLApplet(void){}

public:
    static void InitPath(const char * appletPath);
    void parse();

    inline string description(){ return description_; }
    inline string sql(){ return sqlSource_; }
    inline string pathFile(){ return appletPath_; }
    inline string data(){ return paramValueXmlData_; }
    inline vector<DataInfo>& params(){return params_;}

private:
    vector<DataInfo> params_;
    map<string, string> formattedParamValueList_;

    string appletPath_;
    string paramValueXmlData_;

    string description_;
    string sqlSource_;

};

#endif // SQLAPPLET_H
