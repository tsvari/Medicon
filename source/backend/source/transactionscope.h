#ifndef TRANSACTIONSCOPE_H
#define TRANSACTIONSCOPE_H

#include "sqlconnection.h"

/**
 * @brief RAII transaction scope with automatic rollback on exception
 *
 * Ensures that a database transaction is always properly committed or
 * rolled back, eliminating the manual try/catch/rollback boilerplate.
 *
 * Usage:
 * @code
 * SqlConnection conn(SA_PostgreSQL_Client, host, user, pass);
 * conn.connect();
 *
 * {
 *     TransactionScope tx(conn);
 *     // ... do SQL work ...
 *     tx.commit();  // On success
 * } // Auto-rollback if commit() was not called (e.g., on exception)
 * @endcode
 */
class TransactionScope
{
public:
    /**
     * @brief Begin a transaction on the given connection
     * @param conn The database connection to manage
     *
     * Sets auto-commit off to start a transaction.
     * The connection must already be established (connect() called).
     */
    explicit TransactionScope(SqlConnection& conn) noexcept
        : m_conn(&conn)
    {
        try {
            m_conn->setAutoCommit(false);
        } catch (...) {
            // If setting auto-commit fails, the transaction is not active
            m_conn = nullptr;
        }
    }

    /**
     * @brief Destructor — auto-rollback if not committed
     *
     * If commit() was not called (e.g., due to an exception),
     * automatically rolls back the transaction.
     * Never throws.
     */
    ~TransactionScope() noexcept
    {
        if (m_conn && !m_committed) {
            try {
                m_conn->rollback();
            } catch (...) {
                // Swallow — destructors must not throw
            }
        }
    }

    // No copy or move
    TransactionScope(const TransactionScope&) = delete;
    TransactionScope& operator=(const TransactionScope&) = delete;
    TransactionScope(TransactionScope&&) = delete;
    TransactionScope& operator=(TransactionScope&&) = delete;

    /**
     * @brief Commit the transaction
     * @throws SAException if commit fails
     *
     * Marks the transaction as committed so the destructor won't rollback.
     */
    void commit()
    {
        if (m_conn) {
            m_conn->commit();
            m_committed = true;
        }
    }

    /**
     * @brief Force a rollback
     *
     * Can be called explicitly to rollback before the destructor runs.
     */
    void rollback()
    {
        if (m_conn && !m_committed) {
            m_conn->rollback();
        }
    }

    /**
     * @brief Check if the transaction is still active (not committed)
     */
    [[nodiscard]] bool isActive() const noexcept
    {
        return m_conn && !m_committed;
    }

private:
    SqlConnection* m_conn = nullptr;
    bool m_committed = false;
};

#endif // TRANSACTIONSCOPE_H
