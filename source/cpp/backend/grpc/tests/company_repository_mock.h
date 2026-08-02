#pragma once

#include "company/company_repository.h"
#include <map>
#include <vector>

/**
 * @brief Mock CompanyRepository for unit testing CompanyService
 *
 * Stores data in memory instead of a database.
 * Allows pre-populating data and verifying interactions.
 */
class MockCompanyRepository : public CompanyRepository {
public:
    MockCompanyRepository()
        : CompanyRepository(m_dummyConn, "")
    {
    }

    // Pre-populate the mock with data
    void addPreExisting(const CompanyData& data)
    {
        CompanyData d = data;
        if (d.uid.empty()) {
            d.uid = "pre-" + std::to_string(m_nextPreId++);
        }
        m_storage[d.uid] = d;
    }

    // Retrieve stored data for verification
    const std::map<std::string, CompanyData>& storage() const { return m_storage; }
    int addCount() const { return m_addCount; }
    int updateCount() const { return m_updateCount; }
    int removeCount() const { return m_removeCount; }

    // Overrides
    CompanyData add(const CompanyData& data) override
    {
        ++m_addCount;
        CompanyData result = data;
        if (result.uid.empty()) {
            result.uid = "mock-uid-" + std::to_string(m_addCount);
        }
        m_storage[result.uid] = result;
        return result;
    }

    CompanyData update(const CompanyData& data) override
    {
        ++m_updateCount;
        m_storage[data.uid] = data;
        return data;
    }

    DeleteResult remove(std::string_view uid) override
    {
        ++m_removeCount;
        DeleteResult result;
        auto it = m_storage.find(std::string(uid));
        if (it != m_storage.end()) {
            result.uid = it->second.uid;
            result.success = true;
            m_storage.erase(it);
        } else {
            result.success = false;
            result.error = "Not found";
        }
        return result;
    }

    std::vector<CompanyData> query(const CompanyFilter& filter) override
    {
        std::vector<CompanyData> results;
        for (const auto& [uid, data] : m_storage) {
            // Simple filter: match name if FILTER_VALUE is set
            if (filter.value.empty() ||
                data.name.find(filter.value) != std::string::npos) {
                results.push_back(data);
            }
        }
        // Pagination
        size_t start = static_cast<size_t>(filter.offset);
        size_t limit = static_cast<size_t>(filter.limit);
        size_t end = (start + limit < results.size()) ? (start + limit) : results.size();
        if (start < results.size()) {
            return std::vector<CompanyData>(results.begin() + start,
                                            results.begin() + end);
        }
        return {};
    }

    std::optional<CompanyData> findByUid(std::string_view uid) override
    {
        auto it = m_storage.find(std::string(uid));
        if (it != m_storage.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    int64_t count(const CompanyFilter& filter) override
    {
        int64_t cnt = 0;
        for (const auto& [uid, data] : m_storage) {
            if (filter.value.empty() ||
                data.name.find(filter.value) != std::string::npos) {
                ++cnt;
            }
        }
        return cnt;
    }

private:
    // Dummy connection (never used, overrides avoid calling it, but base ctor needs valid params)
    SqlConnection m_dummyConn = SqlConnection(
        SA_PostgreSQL_Client, "mockhost", "mockuser", "mockpass");

    std::map<std::string, CompanyData> m_storage;
    int m_nextPreId = 0;
    int m_addCount = 0;
    int m_updateCount = 0;
    int m_removeCount = 0;
};
