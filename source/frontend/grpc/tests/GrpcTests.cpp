#include "gtest/gtest.h"
#include "../company_client.hpp"

TEST(ConfigFileIntegrationTests, LoadAndCheckData)
{
    CompanyEditorClient client(grpc::CreateChannel("127.0.0.1:12345", grpc::InsecureChannelCredentials()));

    Company company;
    company.set_uid("11");
    CompanyResult result;

    client.AddCompany(company, result);

    EXPECT_EQ(result.uid(), company.uid());
    EXPECT_TRUE(result.success());
}





