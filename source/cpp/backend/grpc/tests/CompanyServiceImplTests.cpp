/**
 * @file CompanyServiceImplTests.cpp
 * @brief Tests for CompanyServiceImpl protobuf ↔ domain conversions
 *
 * Verifies the pure conversion helpers:
 * - toCompanyData():  protobuf Company → domain CompanyData
 * - toCompanyFilter(): JsonParameters → domain CompanyFilter
 * - toProto():         domain CompanyData → protobuf Company
 *
 * No database or gRPC server needed — these are pure static functions.
 */
#include "company/company_server.h"
#include "gtest/gtest.h"

#include <chrono>
#include <string>

using namespace CompanyEdit;

// ============================================================================
// toCompanyData — protobuf → domain
// ============================================================================

TEST(CompanyServiceImplTest, ToCompanyData_AllFieldsMapped)
{
    Company proto;
    proto.set_uid("uid-1");
    proto.set_server_uid(42);
    proto.set_company_type(3);
    proto.set_name("Acme Corp");
    proto.set_address("123 Main St");
    proto.set_reg_date(1577836800000LL);  // 2020-01-01
    proto.set_joint_date(1593561600000LL); // 2020-07-01
    proto.set_license("LIC-001");
    proto.set_logo("logo-bytes");

    CompanyData data = CompanyServiceImpl::toCompanyData(proto);

    EXPECT_EQ(data.uid, "uid-1");
    EXPECT_EQ(data.server_uid, 42);
    EXPECT_EQ(data.company_type, 3);
    EXPECT_EQ(data.name, "Acme Corp");
    EXPECT_EQ(data.address, "123 Main St");
    EXPECT_EQ(data.reg_date, std::chrono::milliseconds(1577836800000LL));
    EXPECT_EQ(data.joint_date, std::chrono::milliseconds(1593561600000LL));
    EXPECT_EQ(data.license, "LIC-001");
    EXPECT_EQ(data.logo, "logo-bytes");
}

TEST(CompanyServiceImplTest, ToCompanyData_EmptyProto_ProducesDefaults)
{
    Company proto;  // all defaults

    CompanyData data = CompanyServiceImpl::toCompanyData(proto);

    EXPECT_TRUE(data.uid.empty());
    EXPECT_EQ(data.server_uid, 0);
    EXPECT_EQ(data.company_type, 0);
    EXPECT_TRUE(data.name.empty());
    EXPECT_EQ(data.reg_date.count(), 0);
    EXPECT_EQ(data.joint_date.count(), 0);
}

// ============================================================================
// toProto — domain → protobuf
// ============================================================================

TEST(CompanyServiceImplTest, ToProto_AllFieldsMapped)
{
    CompanyData data;
    data.uid = "uid-2";
    data.server_uid = 7;
    data.company_type = 1;
    data.name = "Beta LLC";
    data.address = "456 Oak Ave";
    data.reg_date = std::chrono::milliseconds(1609459200000LL);  // 2021-01-01
    data.joint_date = std::chrono::milliseconds(1625097600000LL); // 2021-07-01
    data.license = "LIC-002";
    data.logo = "logo-bytes-2";

    Company proto;
    CompanyServiceImpl::toProto(data, &proto);

    EXPECT_EQ(proto.uid(), "uid-2");
    EXPECT_EQ(proto.server_uid(), 7);
    EXPECT_EQ(proto.company_type(), 1);
    EXPECT_EQ(proto.name(), "Beta LLC");
    EXPECT_EQ(proto.address(), "456 Oak Ave");
    EXPECT_EQ(proto.reg_date(), 1609459200000LL);
    EXPECT_EQ(proto.joint_date(), 1625097600000LL);
    EXPECT_EQ(proto.license(), "LIC-002");
    EXPECT_EQ(proto.logo(), "logo-bytes-2");
}

TEST(CompanyServiceImplTest, ToProto_EmptyData_ProducesEmptyProto)
{
    CompanyData data;  // all defaults

    Company proto;
    proto.set_name("should-be-cleared");
    CompanyServiceImpl::toProto(data, &proto);

    EXPECT_TRUE(proto.uid().empty());
    EXPECT_EQ(proto.server_uid(), 0);
    EXPECT_TRUE(proto.name().empty());
    EXPECT_TRUE(proto.address().empty());
    EXPECT_TRUE(proto.logo().empty());
}

// ============================================================================
// toCompanyFilter — JsonParameters → domain filter
// ============================================================================

TEST(CompanyServiceImplTest, ToCompanyFilter_AllFieldsParsed)
{
    JsonParameters params;
    params.set_jsonparams(
        "{\"FILTER_FIELD\":\"NAME\",\"FILTER_VALUE\":\"Acme\","
        "\"OFFSET\":\"10\",\"LIMIT\":\"25\"}");

    CompanyFilter filter = CompanyServiceImpl::toCompanyFilter(params);

    EXPECT_EQ(filter.field, "NAME");
    EXPECT_EQ(filter.value, "Acme");
    EXPECT_EQ(filter.offset, 10);
    EXPECT_EQ(filter.limit, 25);
}

TEST(CompanyServiceImplTest, ToCompanyFilter_EmptyParams_ProducesDefaults)
{
    JsonParameters params;  // empty jsonparams — must NOT throw

    CompanyFilter filter = CompanyServiceImpl::toCompanyFilter(params);

    EXPECT_TRUE(filter.field.empty());
    EXPECT_TRUE(filter.value.empty());
    EXPECT_EQ(filter.offset, 0);
    EXPECT_EQ(filter.limit, 100);  // default limit
}

TEST(CompanyServiceImplTest, ToCompanyFilter_InvalidJson_ProducesDefaults)
{
    JsonParameters params;
    params.set_jsonparams("{ not valid json");  // malformed — must NOT throw

    CompanyFilter filter = CompanyServiceImpl::toCompanyFilter(params);

    EXPECT_TRUE(filter.field.empty());
    EXPECT_TRUE(filter.value.empty());
    EXPECT_EQ(filter.offset, 0);
    EXPECT_EQ(filter.limit, 100);  // default limit
}

TEST(CompanyServiceImplTest, ToCompanyFilter_PartialParams_OnlySetFields)
{
    JsonParameters params;
    params.set_jsonparams("{\"FILTER_VALUE\":\"search\"}");

    CompanyFilter filter = CompanyServiceImpl::toCompanyFilter(params);

    EXPECT_TRUE(filter.field.empty());
    EXPECT_EQ(filter.value, "search");
    EXPECT_EQ(filter.offset, 0);
    EXPECT_EQ(filter.limit, 100);  // default limit when not specified
}
