#include "gtest/gtest.h"
#include "../company_client.hpp"
#include "TypeToStringFormatter.h"
#include "include_frontend_util.h"

#include <QDateTime>

// SELECT COUNT(*) FROM my_table;
// SELECT * FROM my_table ORDER BY some_column OFFSET 45 LIMIT 10;

using FrontConverter::to_str;

TEST(ConfigFileIntegrationTests, LoadAndCheckData)
{
    auto compareObjects = [=] (Company & left, Company & right) {
        EXPECT_EQ(left.uid(), right.uid());
        EXPECT_EQ(left.server_uid(), right.server_uid());
        EXPECT_EQ(left.company_type(), right.company_type());
        EXPECT_EQ(left.name(), right.name());
        EXPECT_EQ(left.address(), right.address());
        EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.reg_date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.reg_date(), DataInfo::Date));
        EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.joint_date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.joint_date(), DataInfo::Date));
        EXPECT_EQ(left.license(), right.license());
        EXPECT_EQ(left.logo(), right.logo());
    };

    QDateTime current = QDateTime::currentDateTime();

    Company companyToSend;
    companyToSend.set_server_uid(1);
    companyToSend.set_company_type(0);
    companyToSend.set_name("Givi - გივი");
    companyToSend.set_address("134 George St, New Brunswick, NJ 08901");
    companyToSend.set_reg_date(current.toSecsSinceEpoch());
    companyToSend.set_joint_date(current.toSecsSinceEpoch());
    companyToSend.set_license("0123456789");

    CompanyResult result;
    CompanyEditorClient client(grpc::CreateChannel("127.0.0.1:12345", grpc::InsecureChannelCredentials()));

    CompanyUid companyUid;
    Status status;

    // Check empty result set
    companyUid.set_uid("00000");
    Company emptyCompany;
    status = client.QueryCompanyByUid(companyUid, emptyCompany);
    EXPECT_TRUE(status.error_code() == StatusCode::CANCELLED);

    // Check insert operation
    status = client.AddCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    companyToSend.set_uid(result.uid());

    EXPECT_TRUE(result.uid().size() > 0);
    EXPECT_TRUE(result.success());

    companyUid.set_uid(result.uid());

    Company companyInserted;
    status = client.QueryCompanyByUid(companyUid, companyInserted);

    EXPECT_TRUE(status.ok());
    compareObjects(companyToSend, companyInserted);

    // Update Inserted company/row
    companyToSend.set_name("Givi Tsvariani");
    status = client.EditCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    Company companyEdited;
    companyUid.set_uid(result.uid());
    status = client.QueryCompanyByUid(companyUid, companyEdited);

    EXPECT_TRUE(status.ok());
    compareObjects(companyToSend, companyEdited);

    // Delete Inserted and Updated company/row
}


// qint64 QDateTime::toSecsSinceEpoch() const

// QDateTime QDateTime::fromSecsSinceEpoch(qint64 secs, const QTimeZone &timeZone)
