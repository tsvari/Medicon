#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <SQLAPI.h>

namespace {
const char * SQL_CONNECTION_ERR_INIT = "Connection data: host, user or password were not initialized!";
}
class SqlConnection
{
public:
    SqlConnection();
    SqlConnection(eSAClient client, const char * host, const char * user, const char * pass);
    ~SqlConnection();

    static void InitAllConnections(eSAClient client, const char * host, const char * user, const char * pass);

    void connect();
    void rollback();
    void commit();

    SAConnection * connectionSa(){return & db_con;}
    void setAutoCommit(bool autoCommit);

private:
    SAString  db_host;
    SAString  db_user;
    SAString  db_pass;

    SAConnection db_con;
};

#endif // SQLCONNECTION_H
