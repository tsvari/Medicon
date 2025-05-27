#include "gtest/gtest.h"
#include "../company_client.hpp"
#include "TypeToStringFormatter.h"

#include <QDateTime>

TEST(ConfigFileIntegrationTests, LoadAndCheckData)
{
    CompanyEditorClient client(grpc::CreateChannel("127.0.0.1:12345", grpc::InsecureChannelCredentials()));

    QDateTime current = QDateTime::currentDateTime();
    Company company;
    company.set_server_uid(1);
    company.set_company_type(0);
    company.set_name("Givi");
    company.set_address("134 George St, New Brunswick, NJ 08901");
    company.set_reg_date(current.toSecsSinceEpoch());
    company.set_joint_date(current.toSecsSinceEpoch());
    company.set_license("0123456789");

    CompanyResult result;

    client.AddCompany(company, result);

    EXPECT_TRUE(result.uid().size() > 0);
    EXPECT_TRUE(result.success());
}


// qint64 QDateTime::toSecsSinceEpoch() const

// QDateTime QDateTime::fromSecsSinceEpoch(qint64 secs, const QTimeZone &timeZone)
