/**
 * @file sqlconnection.cpp
 * @brief Implementation of SQL database connection wrapper
 * 
 * This file implements the SqlConnection class methods.
 * No global state — all credentials are per-instance.
 * 
 * @version 3.0
 * @date 2026-08-02
 */

#include "sqlconnection.h"
#include <sstream>
#include <stdexcept>

/**
 * Parameterized constructor implementation.
 * Creates a connection with explicit per-instance credentials.
 */
SqlConnection::SqlConnection(eSAClient client, const char * host, const char * user, const char * pass)
{
    // Validate input parameters
    if (!host || !user || !pass || 
        std::string(host).empty() || std::string(user).empty() || std::string(pass).empty()) {
        throw std::invalid_argument(SQL_CONNECTION_ERR_INVALID);
    }
    
    // Set client type and credentials
    db_client = client;
    db_host = host;
    db_user = user;
    db_pass = pass;
    
    // Configure the database client type
    db_con.setClient(db_client);
}

/**
 * Destructor implementation.
 * Ensures database connection is properly closed.
 * Never throws exceptions (noexcept guarantee).
 */
SqlConnection::~SqlConnection() noexcept
{
    try {
        disconnect();
    } catch (...) {
        // Suppress all exceptions in destructor
        // Logging could be added here if needed
    }
}

/**
 * Move constructor implementation.
 * Transfers ownership of the database connection.
 * Note: SAConnection doesn't support move, so we only move the credentials.
 */
SqlConnection::SqlConnection(SqlConnection&& other)
    : db_client(other.db_client)
{
    // Copy credentials from source (SAString doesn't have noexcept move)
    db_host = other.db_host;
    db_user = other.db_user;
    db_pass = other.db_pass;
    
    // Configure client type for new connection
    db_con.setClient(db_client);
    
    // Leave source in valid but unspecified state
    other.db_client = SA_Client_NotSpecified;
}

/**
 * Move assignment operator implementation.
 * Transfers ownership after cleaning up current state.
 * Note: SAConnection doesn't support move, so we disconnect and transfer credentials only.
 */
SqlConnection& SqlConnection::operator=(SqlConnection&& other)
{
    if (this != &other) {
        // Disconnect current connection if active
        disconnect();
        
        // Transfer ownership of credentials (SAString doesn't have noexcept move)
        db_host = other.db_host;
        db_user = other.db_user;
        db_pass = other.db_pass;
        db_client = other.db_client;
        
        // Reconfigure connection with new client type
        db_con.setClient(db_client);
        
        // Leave source in valid but unspecified state
        other.db_client = SA_Client_NotSpecified;
    }
    return *this;
}

/**
 * Establish database connection.
 * Disconnects first if already connected to ensure clean state.
 */
void SqlConnection::connect()
{
    // Validate credentials before attempting connection
    validateCredentials();
    
    // Ensure we're not already connected (cleanup if needed)
    if (db_con.isConnected()) {
        disconnect();
    }
    
    // Establish new connection with stored credentials
    // SAException will be thrown if connection fails
    db_con.Connect(db_host, db_user, db_pass);
}

/**
 * Disconnect from database.
 * Safe to call even if not connected.
 */
void SqlConnection::disconnect() noexcept
{
    try {
        if (db_con.isConnected()) {
            db_con.Disconnect();
        }
    } catch (...) {
        // Suppress exceptions - disconnect should never fail in a way that matters
        // Logging could be added here if needed
    }
}

/**
 * Check if currently connected.
 */
bool SqlConnection::isConnected() const noexcept
{
    try {
        return db_con.isConnected();
    } catch (...) {
        // If checking connection state throws, assume not connected
        return false;
    }
}

/**
 * Rollback current transaction.
 * Discards all uncommitted changes in the current transaction.
 */
void SqlConnection::rollback()
{
    if (!isConnected()) {
        throw std::runtime_error(SQL_CONNECTION_ERR_NOT_CONNECTED);
    }
    
    // SAException will be thrown if rollback fails
    db_con.Rollback();
}

/**
 * Commit current transaction.
 * Makes all changes in the current transaction permanent.
 */
void SqlConnection::commit()
{
    if (!isConnected()) {
        throw std::runtime_error(SQL_CONNECTION_ERR_NOT_CONNECTED);
    }
    
    // SAException will be thrown if commit fails
    db_con.Commit();
}

/**
 * Configure auto-commit behavior.
 * When enabled, each SQL statement is committed immediately.
 * When disabled, explicit commit() or rollback() calls are required.
 */
void SqlConnection::setAutoCommit(bool autoCommit)
{
    if (autoCommit) {
        // Enable auto-commit: each statement is committed automatically
        db_con.setAutoCommit(SA_AutoCommitOn);
    } else {
        // Disable auto-commit: requires explicit commit()/rollback()
        db_con.setAutoCommit(SA_AutoCommitOff);
    }
}

/**
 * Get connection information string for logging/debugging.
 * Password is masked for security.
 */
std::string SqlConnection::getConnectionInfo() const
{
    std::ostringstream oss;
    
    // Convert SAString to std::string safely
    std::string host_str = db_host.IsEmpty() ? "(empty)" : std::string(db_host.GetMultiByteChars());
    std::string user_str = db_user.IsEmpty() ? "(empty)" : std::string(db_user.GetMultiByteChars());
    
    // Format: user@host (client_type) [connected/disconnected]
    oss << user_str << "@" << host_str;
    
    // Add client type if known
    const char* client_name = "Unknown";
    switch (db_client) {
        case SA_Oracle_Client: client_name = "Oracle"; break;
        case SA_SQLServer_Client: client_name = "SQL Server"; break;
        case SA_MySQL_Client: client_name = "MySQL"; break;
        case SA_PostgreSQL_Client: client_name = "PostgreSQL"; break;
        case SA_SQLite_Client: client_name = "SQLite"; break;
        default: break;
    }
    oss << " (" << client_name << ")";
    
    // Add connection status
    oss << " [" << (isConnected() ? "connected" : "disconnected") << "]";
    
    return oss.str();
}

/**
 * Validate that connection credentials are not empty.
 */
void SqlConnection::validateCredentials() const
{
    if (db_host.IsEmpty() || db_user.IsEmpty() || db_pass.IsEmpty()) {
        throw std::invalid_argument(SQL_CONNECTION_ERR_INVALID);
    }
}
