#include "configfile.h"
#include "sqlconnection.h"

SQLConnection::SQLConnection():
    driver_(NULL),
    con_(NULL)
{

}

SQLConnection::SQLConnection(const ConfigFile& config):
    con_(NULL)
{
    SetConfigFile(config);
}

SQLConnection::~SQLConnection()
{
    Close();
}


void SQLConnection::SetConfigFile(const ConfigFile& config)
{
    config.GetValue("host", host_);
    config.GetValue("user", user_);
    config.GetValue("pass", pass_);
    config.GetValue("schema", database_);
    //config.GetValue("applet-path", applet_path_);
    applet_path_ = config.GetAppletPath();

    driver_ = get_driver_instance();
}

void SQLConnection::Connect()
{
    if(!driver_)
        throw sql::SQLException("Not any driver instance!");

    con_ = driver_->connect(host_, user_, pass_);
    con_->setSchema(database_);
}

void SQLConnection::Close()
{
    if(con_)
        {
        con_->close();
        delete con_;
        con_ = NULL;
        }
}
