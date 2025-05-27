#include "sqlconnection.h"
#include "configfile.h"
#include <stdexcept>

namespace {
static eSAClient g_client;

static SAString  g_host;
static SAString  g_user;
static SAString  g_pass;
}
SqlConnection::SqlConnection()
{
    if(g_host.IsEmpty() || g_user.IsEmpty() || g_pass.IsEmpty()) {
        throw std::runtime_error(SQL_CONNECTION_ERR_INIT);
    }
    db_host = g_host;
    db_user = g_user;
    db_pass = g_pass;

    db_con.setClient(g_client);
}

SqlConnection::SqlConnection(eSAClient client, const char * host, const char * user, const char * pass)
    : db_host(host)
    , db_user(user)
    , db_pass(pass)
{
    db_con.setClient(client);
}

SqlConnection::~SqlConnection()
{
    if(db_con.isConnected()) {
        db_con.Disconnect();
    }
}

void SqlConnection::InitAllConnections(eSAClient client, const char * host, const char * user, const char * pass)
{
    g_client = client;

    g_host = host;
    g_user = user;
    g_pass = pass;
}

void SqlConnection::connect()
{
    if(db_con.isConnected()) {
        db_con.Disconnect();
    }
    db_con.Connect(db_host, db_user, db_pass);
}

void SqlConnection::rollback()
{
    db_con.Rollback();
}

void SqlConnection::commit()
{
    db_con.Commit();
}

void SqlConnection::setAutoCommit(bool autoCommit)
{
    if(autoCommit) {
        db_con.setAutoCommit(SA_AutoCommitOn);
    } else {
        db_con.setAutoCommit(SA_AutoCommitOff);
    }
}
