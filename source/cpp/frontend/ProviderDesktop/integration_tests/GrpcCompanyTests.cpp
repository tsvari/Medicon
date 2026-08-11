// ============================================================================
// CompanyEditorClient end-to-end tests (gRPC -> running provider)
//
// These tests drive the REAL frontend gRPC client (company_client.hpp) against
// a live backend:
//   - a running provider.exe listening on channelAddress (see front_common.h)
//   - PostgreSQL with the company table
//
// A self-contained variant that starts the gRPC server IN-PROCESS (no separate
// provider.exe process) lives in the backend project:
//   backend/grpc/tests/company/integration/CompanyServerIntegrationTests.cpp
// ============================================================================

#include "gtest/gtest.h"
#include "company_client.hpp"
#include "JsonParameterFormatter.h"
#include "TypeToStringFormatter.h"
#include "include_frontend_util.h"

#include "front_common.h"

#include <QDateTime>

using FrontConverter::to_str;
using CommonUtil::sqlRowOffset;
using std::string;

namespace {

// Dedicated owner id for test rows - never touches real data and lets the
// fixture clean up leftovers by SERVER_UID (the production server uses 1001).
constexpr int TEST_SERVER_UID = 30002;

const string logoPath = string(TEST_DATA_DIR) + "logo.png";
const string logoEditPath = string(TEST_DATA_DIR) + "logo-edit.jpg";

// Assert a gRPC call succeeded, surfacing the server error text on failure.
#define EXPECT_GRPC_OK(status) \
    EXPECT_TRUE((status).ok()) << "gRPC failed: " << (status).error_message()

void expectCompanyEqual(const Company& expected, const Company& actual)
{
    EXPECT_EQ(actual.uid(), expected.uid());
    EXPECT_EQ(actual.server_uid(), expected.server_uid());
    EXPECT_EQ(actual.company_type(), expected.company_type());
    EXPECT_EQ(actual.name(), expected.name());
    EXPECT_EQ(actual.address(), expected.address());
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(actual.reg_date(), DataInfo::Date),
              TimeFormatHelper::chronoSysSecToString(expected.reg_date(), DataInfo::Date));
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(actual.joint_date(), DataInfo::Date),
              TimeFormatHelper::chronoSysSecToString(expected.joint_date(), DataInfo::Date));
    EXPECT_EQ(actual.license(), expected.license());
    EXPECT_EQ(actual.logo(), expected.logo());
}

// Fill the standard company fields with realistic test data.
void fillCompany(Company& c, const std::string& name)
{
    QDateTime current = QDateTime::currentDateTime();
    c.set_server_uid(TEST_SERVER_UID);
    c.set_company_type(0);
    c.set_name(name);
    c.set_address("134 George St, New Brunswick, NJ 08901");
    c.set_reg_date(current.toSecsSinceEpoch());
    c.set_joint_date(current.toSecsSinceEpoch());
    c.set_license("0123456789");
}

} // namespace

// ============================================================================
// Fixture: one gRPC client per test
// ============================================================================

class CompanyEditorTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        m_client = std::make_unique<CompanyEditorClient>(
            grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));
    }

    CompanyEditorClient& client() { return *m_client; }

    // Add a company and return its uid; aborts the test on failure.
    std::string addCompany(Company& c)
    {
        CompanyResult result;
        Status s = client().AddCompany(c, result);
        EXPECT_TRUE(s.ok()) << "AddCompany gRPC failed: " << s.error_message();
        EXPECT_TRUE(result.success()) << "AddCompany reported failure: " << result.error();
        EXPECT_FALSE(result.uid().empty());
        return result.uid();
    }

    void TearDown() override
    {
        // Delete any rows left by this test (also runs when the test aborts),
        // so leftovers never accumulate and break the count assertions.
        if (!m_client) {
            return;
        }
        JsonParameterFormatter f;
        f.addParameter("SERVER_UID", TEST_SERVER_UID);
        f.addParameter("FILTER_FIELD", "NAME");
        f.addParameter("FILTER_VALUE", "");
        f.addParameter("OFFSET", 0);
        f.addParameter("LIMIT", 1000);
        JsonParameters params;
        params.set_jsonparams(f.toJson());
        std::vector<Company> leftover;
        if (m_client->QueryCompanies(params, leftover).ok()) {
            for (Company& c : leftover) {
                CompanyResult r;
                m_client->DeleteCompany(c, r);
            }
        }
    }

    std::unique_ptr<CompanyEditorClient> m_client;
};

// ============================================================================
// Full CRUD round-trip over gRPC
// ============================================================================

TEST_F(CompanyEditorTest, LoadAndCheckData)
{
    Company companyToSend;
    fillCompany(companyToSend, "Givi - გივი");

    std::string uid = addCompany(companyToSend);
    companyToSend.set_uid(uid);

    // Read back and compare every field
    CompanyUid companyUid;
    companyUid.set_uid(uid);
    Company inserted;
    EXPECT_GRPC_OK(client().QueryCompanyByUid(companyUid, inserted));
    expectCompanyEqual(companyToSend, inserted);

    // Update
    companyToSend.set_name("Givi Tsvariani");
    CompanyResult result;
    EXPECT_GRPC_OK(client().EditCompany(companyToSend, result));
    ASSERT_FALSE(result.uid().empty());

    Company edited;
    companyUid.set_uid(result.uid());
    EXPECT_GRPC_OK(client().QueryCompanyByUid(companyUid, edited));
    expectCompanyEqual(companyToSend, edited);

    // Delete, then verify the row is gone
    EXPECT_GRPC_OK(client().DeleteCompany(companyToSend, result));
    Company gone;
    Status s = client().QueryCompanyByUid(companyUid, gone);
    EXPECT_EQ(s.error_code(), StatusCode::NOT_FOUND)
        << "expected NOT_FOUND after delete, got: " << s.error_message();
}

// ============================================================================
// Count + paginated select with an empty filter (match-all)
// ============================================================================

TEST_F(CompanyEditorTest, SelectCompanieTests)
{
    const int rows = 22;
    std::vector<std::string> uidList;
    uidList.reserve(rows);

    for (int i = 0; i < rows; ++i) {
        Company c;
        fillCompany(c, QString("Givi - %1").arg(i).toStdString());
        uidList.push_back(addCompany(c));
    }

    // Count with an empty filter value -> must match all inserted rows
    JsonParameterFormatter jsonFormatter;
    jsonFormatter.addParameter("SERVER_UID", TEST_SERVER_UID);
    jsonFormatter.addParameter("FILTER_FIELD", "NAME");
    jsonFormatter.addParameter("FILTER_VALUE", "");

    TotalCount totalCount;
    JsonParameters parametersCount;
    parametersCount.set_jsonparams(jsonFormatter.toJson());
    EXPECT_GRPC_OK(client().QueryCompanyTotalCount(parametersCount, totalCount));
    ASSERT_EQ(totalCount.count(), rows);

    // Page 2, 4 per page -> offset 4
    int page = 2;
    int limitationPerPage = 4;
    int offset = sqlRowOffset(page, limitationPerPage, totalCount.count());
    EXPECT_EQ(offset, 4);

    JsonParameters parametersSelect;
    jsonFormatter.addParameter("OFFSET", offset);
    jsonFormatter.addParameter("LIMIT", limitationPerPage);
    parametersSelect.set_jsonparams(jsonFormatter.toJson());
    std::vector<Company> object_list;
    EXPECT_GRPC_OK(client().QueryCompanies(parametersSelect, object_list));
    ASSERT_EQ(object_list.size(), static_cast<size_t>(limitationPerPage));

    EXPECT_EQ(object_list.at(0).name(), QString("Givi - 4"));
    EXPECT_EQ(object_list.at(limitationPerPage - 1).name(), QString("Givi - 7"));

    // Clean up
    for (const auto& uid : uidList) {
        Company c;
        c.set_uid(uid);
        CompanyResult result;
        EXPECT_GRPC_OK(client().DeleteCompany(c, result));
    }
}

// ============================================================================
// bytea logo round-trip over gRPC
// ============================================================================

TEST_F(CompanyEditorTest, LogoBinaryRoundTrip)
{
    std::string logoString;
    ASSERT_NO_THROW(logoString = StdBinary::toStdString(logoPath.c_str()));

    Company companyToSend;
    fillCompany(companyToSend, "Logo Company");
    companyToSend.set_logo(logoString);

    std::string uid = addCompany(companyToSend);

    CompanyUid companyUid;
    companyUid.set_uid(uid);

    Company readBack;
    EXPECT_GRPC_OK(client().QueryCompanyByUid(companyUid, readBack));
    EXPECT_EQ(readBack.logo(), logoString)
        << "bytea logo must round-trip byte-for-byte over gRPC";

    // Update with a different image
    std::string logoEditString;
    ASSERT_NO_THROW(logoEditString = StdBinary::toStdString(logoEditPath.c_str()));
    companyToSend.set_logo(logoEditString);
    companyToSend.set_uid(uid);

    CompanyResult result;
    EXPECT_GRPC_OK(client().EditCompany(companyToSend, result));

    EXPECT_GRPC_OK(client().QueryCompanyByUid(companyUid, readBack));
    EXPECT_EQ(readBack.logo(), logoEditString)
        << "updated bytea logo must round-trip byte-for-byte over gRPC";

    // Clean up
    EXPECT_GRPC_OK(client().DeleteCompany(companyToSend, result));
}
