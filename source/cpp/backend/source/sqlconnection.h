/**
 * @file sqlconnection.h
 * @brief Improved SQL database connection wrapper class
 * 
 * This file contains the SqlConnection class which provides an improved,
 * thread-safe RAII wrapper around the SQLAPI++ SAConnection class.
 * 
 * Improvements in this version:
 * - Thread-safe global credential management with mutex protection
 * - Move semantics support (C++11)
 * - Ability to clear global credentials
 * - Better error messages with validation
 * - Deprecation of default constructor (prefers explicit credentials)
 * - Connection state queries
 * 
 * @version 2.0
 * @date 2026-01-05
 */

#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <SQLAPI.h>
#include <string>
#include <mutex>

namespace {
    /// Error message when connection parameters are not initialized
    const char * SQL_CONNECTION_ERR_INIT = "Connection data not initialized! Call InitAllConnections() or use parameterized constructor.";
    
    /// Error message when connection parameters are invalid
    const char * SQL_CONNECTION_ERR_INVALID = "Invalid connection parameters: host, user, and password must not be empty.";
    
    /// Error message when connection is not established
    const char * SQL_CONNECTION_ERR_NOT_CONNECTED = "Operation requires an active database connection.";
}

/**
 * @class SqlConnection
 * @brief Thread-safe RAII wrapper for SQL database connections
 * 
 * The SqlConnection class manages database connections using SQLAPI++ library
 * with enhanced thread safety and security features.
 * 
 * Construction modes:
 * 1. Parameterized constructor (RECOMMENDED) - Explicit credentials per instance
 * 2. Default constructor (DEPRECATED) - Uses global credentials set via InitAllConnections
 * 
 * Thread Safety:
 * - Global credential access is protected by mutex
 * - Each instance should be used by a single thread
 * - Multiple instances can be created safely in different threads
 * 
 * Security Considerations:
 * - Credentials stored in SAString (SQLAPI++ managed)
 * - Use parameterized constructor to avoid global state
 * - Call ClearAllConnections() to erase global credentials when no longer needed
 * 
 * @note Prefer parameterized constructor over default constructor
 * @note Always call ClearAllConnections() before application shutdown
 * 
 * Example usage:
 * @code
 * // Recommended: Explicit credentials
 * SqlConnection conn(SA_MySQL_Client, "localhost", "user", "pass");
 * conn.connect();
 * // ... use connection ...
 * // Automatic cleanup via destructor
 * 
 * // Alternative: Global credentials (thread-safe but not recommended)
 * SqlConnection::InitAllConnections(SA_MySQL_Client, "localhost", "user", "pass");
 * SqlConnection conn;
 * conn.connect();
 * // ... use connection ...
 * SqlConnection::ClearAllConnections(); // Clear sensitive data
 * @endcode
 */
class SqlConnection
{
public:
    /**
     * @brief Default constructor using global connection parameters (DEPRECATED)
     * 
     * Creates a connection object using thread-safe global parameters set by 
     * InitAllConnections(). The connection is not established until connect() is called.
     * 
     * @deprecated Prefer using parameterized constructor for better security
     * @throws std::runtime_error if InitAllConnections was not called first
     * @see InitAllConnections
     * @see SqlConnection(eSAClient, const char*, const char*, const char*)
     */
    SqlConnection();
    
    /**
     * @brief Constructor with explicit connection parameters (RECOMMENDED)
     * 
     * Creates a connection object with specified database credentials.
     * This is the preferred way to create connections as it avoids global state.
     * The connection is not established until connect() is called.
     * 
     * @param client Database client type (e.g., SA_MySQL_Client, SA_PostgreSQL_Client,
     *               SA_SQLServer_Client, SA_Oracle_Client)
     * @param host Database host address (e.g., "localhost", "192.168.1.1:3306")
     * @param user Database username for authentication
     * @param pass Database password for authentication
     * 
     * @throws std::invalid_argument if any parameter is empty/null
     */
    SqlConnection(eSAClient client, const char * host, const char * user, const char * pass);
    
    /**
     * @brief Destructor
     * 
     * Automatically disconnects from the database if still connected.
     * Ensures proper resource cleanup following RAII principles.
     * 
     * @note Never throws exceptions (calls disconnect internally with exception handling)
     */
    ~SqlConnection() noexcept;

    // Disable copy operations (database connections should not be copied)
    SqlConnection(const SqlConnection&) = delete;
    SqlConnection& operator=(const SqlConnection&) = delete;

    // Enable move operations (allows transfer of connection ownership)
    /**
     * @brief Move constructor
     * 
     * Transfers ownership of database connection from another instance.
     * The source instance will be left in a valid but disconnected state.
     * 
     * @param other Source connection to move from
     * @note Not noexcept due to SAString operations
     */
    SqlConnection(SqlConnection&& other);
    
    /**
     * @brief Move assignment operator
     * 
     * Transfers ownership of database connection from another instance.
     * Disconnects current connection if active before taking ownership.
     * 
     * @param other Source connection to move from
     * @return Reference to this instance
     * @note Not noexcept due to SAString operations and disconnect()
     */
    SqlConnection& operator=(SqlConnection&& other);

    /**
     * @brief Initialize global connection parameters (thread-safe)
     * 
     * Sets default connection parameters used by the default constructor.
     * This method is thread-safe and can be called from multiple threads.
     * Must be called before creating SqlConnection objects with default constructor.
     * 
     * @param client Database client type (e.g., SA_MySQL_Client, SA_PostgreSQL_Client)
     * @param host Database host address
     * @param user Database username
     * @param pass Database password
     * 
     * @throws std::invalid_argument if any parameter is empty/null
     * 
     * @note This is a static method affecting all future default-constructed instances
     * @note Thread-safe: Uses mutex to protect global state
     * @warning Global credentials remain in memory until ClearAllConnections() is called
     * 
     * @see ClearAllConnections
     */
    static void InitAllConnections(eSAClient client, const char * host, const char * user, const char * pass);

    /**
     * @brief Clear global connection parameters (thread-safe)
     * 
     * Erases global credentials from memory for security purposes.
     * Should be called when global credentials are no longer needed,
     * typically before application shutdown.
     * 
     * @note Thread-safe: Uses mutex to protect global state
     * @note After calling this, default constructor will throw until InitAllConnections is called again
     */
    static void ClearAllConnections();

    /**
     * @brief Check if global connection parameters are initialized
     * 
     * Determines whether InitAllConnections has been called with valid parameters.
     * 
     * @return true if global credentials are set, false otherwise
     * @note Thread-safe: Uses mutex to protect global state
     */
    static bool IsGloballyInitialized();

    /**
     * @brief Establish connection to the database
     * 
     * Opens a connection using the credentials provided during construction.
     * If already connected, disconnects first then reconnects (ensures clean state).
     * 
     * @throws SAException if connection fails (network error, auth failure, etc.)
     * @throws std::runtime_error if credentials are invalid
     */
    void connect();
    
    /**
     * @brief Disconnect from the database
     * 
     * Closes the active database connection.
     * Safe to call even if not connected (no-op).
     * 
     * @note Does not throw exceptions
     */
    void disconnect() noexcept;
    
    /**
     * @brief Check if currently connected to database
     * 
     * @return true if connection is active, false otherwise
     */
    bool isConnected() const noexcept;
    
    /**
     * @brief Rollback current transaction
     * 
     * Discards all changes made in the current transaction.
     * Should be called when an error occurs during transaction processing.
     * 
     * @throws SAException if rollback fails (e.g., network error)
     * @throws std::runtime_error if not connected
     */
    void rollback();
    
    /**
     * @brief Commit current transaction
     * 
     * Persists all changes made in the current transaction to the database.
     * 
     * @throws SAException if commit fails (e.g., constraint violation, network error)
     * @throws std::runtime_error if not connected
     */
    void commit();

    /**
     * @brief Get pointer to underlying SAConnection object
     * 
     * Provides direct access to the SQLAPI++ connection object.
     * Use this when you need to create SACommand objects or access
     * advanced SQLAPI++ features.
     * 
     * @return Pointer to the internal SAConnection object
     * @note The returned pointer is valid for the lifetime of this object
     */
    SAConnection * connectionSa() noexcept { return &db_con; }
    
    /**
     * @brief Get const pointer to underlying SAConnection object
     * 
     * Const version of connectionSa() for read-only access.
     * 
     * @return Const pointer to the internal SAConnection object
     */
    const SAConnection * connectionSa() const noexcept { return &db_con; }
    
    /**
     * @brief Set auto-commit mode
     * 
     * Controls whether each SQL statement is automatically committed.
     * 
     * @param autoCommit true to enable auto-commit, false to require explicit commit()
     * 
     * @note When auto-commit is off, you must call commit() to persist changes
     *       or rollback() to discard them
     * @note Changing this setting affects subsequent SQL operations
     */
    void setAutoCommit(bool autoCommit);

    /**
     * @brief Get connection string representation (for logging/debugging)
     * 
     * Returns a safe string representation of connection parameters.
     * Password is masked for security.
     * 
     * @return Connection string in format "user@host (client_type)"
     */
    std::string getConnectionInfo() const;

private:
    /**
     * @brief Validate connection parameters
     * 
     * Checks that host, user, and password are not empty.
     * 
     * @throws std::invalid_argument if validation fails
     */
    void validateCredentials() const;

    /// Database host address (e.g., server IP or hostname)
    SAString  db_host;
    
    /// Database username for authentication
    SAString  db_user;
    
    /// Database password for authentication  
    SAString  db_pass;
    
    /// Database client type (MySQL, PostgreSQL, etc.)
    eSAClient db_client;

    /// Underlying SQLAPI++ connection object
    SAConnection db_con;

    // Static members for global connection parameters
    /// Mutex protecting global connection parameters
    static std::mutex g_mutex;
    
    /// Global database client type
    static eSAClient g_client;
    
    /// Global database host address
    static SAString  g_host;
    
    /// Global database username
    static SAString  g_user;
    
    /// Global database password
    static SAString  g_pass;
    
    /// Flag indicating if global parameters are initialized
    static bool g_initialized;
};

#endif // SQLCONNECTION_H
