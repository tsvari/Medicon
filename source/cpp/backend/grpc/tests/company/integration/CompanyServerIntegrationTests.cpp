/**
 * @file CompanyServerIntegrationTests.cpp
 * @brief Self-contained end-to-end tests: gRPC server hosted IN-PROCESS
 *
 * Starts the real CompanyServiceImpl gRPC server inside the test process on an
 * ephemeral port (NO separate provider.exe needed), then drives it through the
 * real frontend gRPC client (CompanyEditorClient). Covers the full stack:
 *
 *   client -> gRPC -> CompanyServiceImpl -> CompanyService (TransactionScope)
 *           -> CompanyRepository -> SqlTemplate -> SQLAPI++ -> PostgreSQL
 *
 * Requires PostgreSQL with the company table. Skipped when:
 *   - MEDICON_SKIP_DB_TESTS is set, or
 *   - the provider config is missing, or
 *   - PostgreSQL / server startup fails
 *
 * Uses a dedicated SERVER_UID (30003) and cleans up all rows it creates.
 */
#include "company/company_server.h"
#include "company/company_service.h"
#include "company_client.hpp"
#include "JsonParameterFormatter.h"
#include "configfile.h"
#include "gtest/gtest.h"

#include <grpcpp/grpcpp.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

namespace {

// Dedicated owner id for test rows (smallint-safe, distinct from the other
// test suites' uids 30001/30002).
constexpr int TEST_SERVER_UID = 30003;
constexpr const char* TEST_LICENSE = "LIC-TEST";

void fillCompany(Company& c, const std::string& name, const std::string& logo = "")
{
    c.set_server_uid(TEST_SERVER_UID);
    c.set_company_type(0);
    c.set_name(name);
    c.set_address("123 Test St");
    c.set_reg_date(1577836800000LL);   // 2020-01-01
    c.set_joint_date(1593561600000LL); // 2020-07-01
    c.set_license(TEST_LICENSE);
    c.set_logo(logo);
}

} // namespace

class CompanyServerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        const char* skip = std::getenv("MEDICON_SKIP_DB_TESTS");
        if (skip && skip[0] != '\0') {
            GTEST_SKIP() << "MEDICON_SKIP_DB_TESTS is set";
        }

        ConfigFile cfg(ALL_PROJECT_APPDATA_PATH, "provider");
        cfg.load();
        m_host = cfg.value("host");
        m_user = cfg.value("user");
        m_pass = cfg.value("pass");
        m_appletPath = cfg.appletPath();
        if (m_host.empty() || m_user.empty() || m_appletPath.empty()) {
            GTEST_SKIP() << "provider DB config incomplete";
        }

        // Build the in-process gRPC server on an ephemeral port
        m_service = std::make_unique<CompanyService>(
            m_appletPath, m_host, m_user, m_pass, /*logSql=*/false);
        m_impl = std::make_unique<CompanyServiceImpl>(std::move(m_service), /*logSql=*/false);

        grpc::ServerBuilder builder;
        int port = 0;
        builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &port);
        builder.RegisterService(m_impl.get());
        m_server = builder.BuildAndStart();
        if (!m_server || port == 0) {
            GTEST_SKIP() << "could not start in-process gRPC server";
        }

        m_client = std::make_unique<CompanyEditorClient>(grpc::CreateChannel(
            "127.0.0.1:" + std::to_string(port), grpc::InsecureChannelCredentials()));
    }

    void TearDown() override
    {
        // Best-effort cleanup of any rows the test left behind (also runs when
        // the test aborts mid-way).
        if (m_client) {
            JsonParameterFormatter f;
            f.addParameter("SERVER_UID", TEST_SERVER_UID);
            f.addParameter("FILTER_FIELD", "NAME");
            f.addParameter("FILTER_VALUE", "");
            f.addParameter("OFFSET", 0);
            f.addParameter("LIMIT", 1000);
            JsonParameters params;
            params.set_jsonparams(f.toJson());
            std::vector<Company> rows;
            if (m_client->QueryCompanies(params, rows).ok()) {
                for (Company& c : rows) {
                    CompanyResult r;
                    m_client->DeleteCompany(c, r);
                }
            }
        }
        if (m_server) {
            m_server->Shutdown();
        }
    }

    std::unique_ptr<CompanyService> m_service;
    std::unique_ptr<CompanyServiceImpl> m_impl;
    std::unique_ptr<grpc::Server> m_server;
    std::unique_ptr<CompanyEditorClient> m_client;
    std::string m_host, m_user, m_pass, m_appletPath;
};

// ============================================================================
// Full CRUD through the real gRPC client against the in-process server
// ============================================================================

TEST_F(CompanyServerIntegrationTest, Add_Query_Update_Delete)
{
    Company toSend;
    fillCompany(toSend, "InProcess Corp");

    CompanyResult result;
    ASSERT_TRUE(m_client->AddCompany(toSend, result).ok())
        << "AddCompany failed: " << result.error();
    ASSERT_FALSE(result.uid().empty());
    toSend.set_uid(result.uid());

    // Read back
    CompanyUid uid;
    uid.set_uid(result.uid());
    Company readBack;
    ASSERT_TRUE(m_client->QueryCompanyByUid(uid, readBack).ok());
    EXPECT_EQ(readBack.name(), "InProcess Corp");
    EXPECT_EQ(readBack.server_uid(), TEST_SERVER_UID);
    EXPECT_EQ(readBack.license(), TEST_LICENSE);

    // Update
    toSend.set_name("InProcess Corp v2");
    ASSERT_TRUE(m_client->EditCompany(toSend, result).ok());
    ASSERT_FALSE(result.uid().empty());
    ASSERT_TRUE(m_client->QueryCompanyByUid(uid, readBack).ok());
    EXPECT_EQ(readBack.name(), "InProcess Corp v2");

    // Delete -> NOT_FOUND
    ASSERT_TRUE(m_client->DeleteCompany(toSend, result).ok());
    auto status = m_client->QueryCompanyByUid(uid, readBack);
    EXPECT_EQ(status.error_code(), StatusCode::NOT_FOUND);
}

// ============================================================================
// Filter regression over the full stack: server_uid + empty filter
// ============================================================================

TEST_F(CompanyServerIntegrationTest, Count_And_Query_WithEmptyFilter)
{
    const int rows = 5;
    for (int i = 0; i < rows; ++i) {
        Company c;
        fillCompany(c, "Filter " + std::to_string(i));
        CompanyResult r;
        ASSERT_TRUE(m_client->AddCompany(c, r).ok());
    }

    JsonParameterFormatter f;
    f.addParameter("SERVER_UID", TEST_SERVER_UID);
    f.addParameter("FILTER_FIELD", "NAME");
    f.addParameter("FILTER_VALUE", "");

    TotalCount totalCount;
    JsonParameters params;
    params.set_jsonparams(f.toJson());
    ASSERT_TRUE(m_client->QueryCompanyTotalCount(params, totalCount).ok());
    EXPECT_EQ(totalCount.count(), rows) << "empty filter must match all rows";

    f.addParameter("OFFSET", 0);
    f.addParameter("LIMIT", 100);
    params.set_jsonparams(f.toJson());
    std::vector<Company> list;
    ASSERT_TRUE(m_client->QueryCompanies(params, list).ok());
    EXPECT_EQ(list.size(), rows);
}

// ============================================================================
// bytea logo round-trip through gRPC
// ============================================================================

TEST_F(CompanyServerIntegrationTest, Logo_RoundTrip_ThroughGrpc)
{
    // PNG-style bytes with embedded NULs and high bytes
    std::string logo;
    logo += "\x89PNG\r\n\x1A\n";
    logo += std::string("\x00\x00\x01", 3);
    logo += "payload\xFF\xFE\x00";

    Company toSend;
    fillCompany(toSend, "Logo Co", logo);

    CompanyResult result;
    ASSERT_TRUE(m_client->AddCompany(toSend, result).ok());
    ASSERT_FALSE(result.uid().empty());

    CompanyUid uid;
    uid.set_uid(result.uid());
    Company readBack;
    ASSERT_TRUE(m_client->QueryCompanyByUid(uid, readBack).ok());
    EXPECT_EQ(readBack.logo(), logo)
        << "bytea logo must round-trip byte-for-byte through gRPC";
}
