#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H


#include <mysql_driver.h>
#include <mysql_connection.h>

//#include <string>
using std::string;

class ConfigFile;

class SQLConnection
{
public:
    SQLConnection();
    explicit SQLConnection(const ConfigFile& config);
    virtual ~SQLConnection();

    void SetConfigFile(const ConfigFile& config);
    void Connect();
    void Close();

    inline sql::Connection* GetConnection() const {return con_;}
    inline const string GetAppletPath() const{return applet_path_;}
private:
    sql::Driver *driver_;
    sql::Connection *con_;

    string host_;
    string user_;
    string pass_;
    string applet_path_;
    string database_;
};

#endif // SQLCONNECTION_H
