/**
 * @file sqlconnection.cpp
 * @brief Implementation of improved SQL database connection wrapper
 * 
 * This file implements the SqlConnection class methods with thread-safe
 * global credential management and enhanced error handling.
 * 
 * @version 2.0
 * @date 2026-01-05
 */

#include "sqlconnection.h"
#include "configfile.h"
#include <stdexcept>
#include <sstream>

// Initialize static members
std::mutex SqlConnection::g_mutex;
eSAClient SqlConnection::g_client = SA_Client_NotSpecified;
SAString SqlConnection::g_host;
SAString SqlConnection::g_user;
SAString SqlConnection::g_pass;
bool SqlConnection::g_initialized = false;

/**
 * Default constructor implementation.
 * Uses thread-safe access to global connection parameters.
 */
SqlConnection::SqlConnection()
{
    eSAClient client_copy;
    
    // Thread-safe access to global parameters
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        
        // Verify global connection parameters were set via InitAllConnections
        if (!g_initialized) {
            throw std::runtime_error(SQL_CONNECTION_ERR_INIT);
        }
        
        // Copy all parameters while holding lock
        client_copy = g_client;
        db_host = g_host;
        db_user = g_user;
        db_pass = g_pass;
    }
    
    // Set client type after copying strings (outside lock)
    db_client = client_copy;
    db_con.setClient(db_client);
}

/**
 * Parameterized constructor implementation.
 * Preferred method for creating connections to avoid global state.
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
 * Initialize global connection parameters (thread-safe).
 * Static method that sets default values used by the default constructor.
 */
void SqlConnection::InitAllConnections(eSAClient client, const char * host, const char * user, const char * pass)
{
    // Validate input parameters
    if (!host || !user || !pass || 
        std::string(host).empty() || std::string(user).empty() || std::string(pass).empty()) {
        throw std::invalid_argument(SQL_CONNECTION_ERR_INVALID);
    }
    
    // Thread-safe update of global parameters
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Set global database client type
    g_client = client;

    // Set global connection credentials
    g_host = host;
    g_user = user;
    g_pass = pass;
    
    // Mark as initialized
    g_initialized = true;
}

/**
 * Clear global connection parameters (thread-safe).
 * Erases sensitive credential data from memory.
 */
void SqlConnection::ClearAllConnections()
{
    // Thread-safe clearing of global parameters
    std::lock_guard<std::mutex> lock(g_mutex);
    
    // Clear all credential data
    g_client = SA_Client_NotSpecified;
    g_host.Empty();
    g_user.Empty();
    g_pass.Empty();
    
    // Mark as uninitialized
    g_initialized = false;
}

/**
 * Check if global credentials are initialized (thread-safe).
 */
bool SqlConnection::IsGloballyInitialized()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_initialized;
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
