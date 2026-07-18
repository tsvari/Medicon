#include "gtest/gtest.h"
#include "company_client.hpp"
#include "TypeToStringFormatter.h"
#include "JsonParameterFormatter.h"
#include "include_frontend_util.h"

#include "front_common.h"

#include <QDateTime>

// SELECT COUNT(*) FROM my_table;
// SELECT * FROM my_table ORDER BY some_column OFFSET 45 LIMIT 10;


using FrontConverter::to_str;
using CommonUtil::sqlRowOffset;

namespace {
const string logoPath = string(TEST_DATA_DIR) + "logo.png";
const string logoToWritePath = string(TEST_DATA_DIR) + "logo2.png";
const string logoEditPath = string(TEST_DATA_DIR) + "logo-edit.jpg";
const string logoEditToWritePath = string(TEST_DATA_DIR) + "logo-edit2.jpg";
void compareObjects (Company & left, Company & right) {
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
}

TEST(CompanyIntegrationTests, LoadAndCheckData)
{
    CompanyEditorClient client(grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));

    QDateTime current = QDateTime::currentDateTime();
    Company companyToSend;
    companyToSend.set_server_uid(1001);
    companyToSend.set_company_type(0);
    companyToSend.set_name("Givi - გივი");
    companyToSend.set_address("134 George St, New Brunswick, NJ 08901");
    companyToSend.set_reg_date(current.toSecsSinceEpoch());
    companyToSend.set_joint_date(current.toSecsSinceEpoch());
    companyToSend.set_license("0123456789");

    CompanyResult result;
    CompanyUid companyUid;
    Status status;

    // Check empty result set
    //companyUid.set_uid("00000");
    //Company emptyCompany;
    //status = client.QueryCompanyByUid(companyUid, emptyCompany);
    //// '00000' isn't uuid so should be issued error
    //EXPECT_TRUE(status.error_code() == StatusCode::CANCELLED);

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

    //// Update Inserted company/row
    companyToSend.set_name("Givi Tsvariani");
    status = client.EditCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    Company companyEdited;
    companyUid.set_uid(result.uid());
    status = client.QueryCompanyByUid(companyUid, companyEdited);
    EXPECT_TRUE(status.ok());

    compareObjects(companyToSend, companyEdited);

    //// Delete Inserted and Updated company/row
    status = client.DeleteCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    companyUid.set_uid(result.uid());
    status = client.QueryCompanyByUid(companyUid, companyEdited);
    //// Should be no row
    EXPECT_TRUE(status.error_code() == StatusCode::NOT_FOUND);
}

TEST(CompanyIntegrationTests, SelectCompanieTests)
{
    CompanyEditorClient client(grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));
    QDateTime current = QDateTime::currentDateTime();

    Company companyToSend;
    companyToSend.set_server_uid(server_uid);
    companyToSend.set_company_type(0);
    companyToSend.set_address("134 George St, New Brunswick, NJ 08901");
    companyToSend.set_reg_date(current.toSecsSinceEpoch());
    companyToSend.set_joint_date(current.toSecsSinceEpoch());
    companyToSend.set_license("0123456789");


    Status status;
    CompanyResult result;
    CompanyUid companyUid;

    const int rows = 22;
    std::vector<std::string> uidList;

    // INsert new rows
    for (int i = 0; i < rows; i++) {

        companyToSend.set_name(QString("Givi - %1").arg(i).toStdString());
        status = client.AddCompany(companyToSend, result);
        EXPECT_TRUE(status.ok());

        companyUid.set_uid(result.uid());
        uidList.push_back(result.uid());
    }
    // Check count
    JsonParameterFormatter jsonFormatter;
    jsonFormatter.AddDataInfo("SERVER_UID", server_uid);
    jsonFormatter.AddDataInfo("FILTER_FIELD", "NAME");
    jsonFormatter.AddDataInfo("FILTER_VALUE", "");

    TotalCount totalCount;
    JsonParameters parametersCount;
    parametersCount.set_jsonparams(jsonFormatter.toJsonString());
    status = client.QueryCompanyTotalCount(parametersCount, totalCount);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(totalCount.count(), rows);

    int page = 2;
    int limitationPerPage = 4;
    int ofset = sqlRowOffset(page, limitationPerPage, totalCount.count());
    EXPECT_EQ(ofset, 4);

    JsonParameters parametersSelect;
    std::vector<Company> object_list;
    jsonFormatter.AddDataInfo("OFFSET", ofset);
    jsonFormatter.AddDataInfo("LIMIT", limitationPerPage);
    parametersSelect.set_jsonparams(jsonFormatter.toJsonString());
    status = client.QueryCompanies(parametersSelect, object_list);
    EXPECT_TRUE(status.ok());
    EXPECT_EQ(object_list.size(), limitationPerPage);

    Company first = object_list.at(0);
    Company last = object_list.at(limitationPerPage - 1);
    EXPECT_EQ(first.name(), QString("Givi - 4"));
    EXPECT_EQ(last.name(), QString("Givi - 7"));

    // Delete all of them
    for (int i = 0; i < rows; i++) {
        companyToSend.set_uid(uidList[i]);
        status = client.DeleteCompany(companyToSend, result);
        EXPECT_TRUE(status.ok());
    }
}

TEST(CompanyLogoTests, SelectCompanieTests)
{
    CompanyEditorClient client(grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));
    QDateTime current = QDateTime::currentDateTime();

    Company companyToSend;
    companyToSend.set_server_uid(server_uid);
    companyToSend.set_company_type(0);
    companyToSend.set_address("134 George St, New Brunswick, NJ 08901");
    companyToSend.set_reg_date(current.toSecsSinceEpoch());
    companyToSend.set_joint_date(current.toSecsSinceEpoch());
    companyToSend.set_license("0123456789");

    Status status;
    CompanyResult result;

    std::string logoString;
    EXPECT_NO_THROW(logoString = StdBinary::toStdString(logoPath.c_str()));
    companyToSend.set_logo(logoString);

    status = client.AddCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    CompanyUid companyUid;
    companyUid.set_uid(result.uid());

    Company companyInsertedEdited;
    status = client.QueryCompanyByUid(companyUid, companyInsertedEdited);
    EXPECT_TRUE(status.ok());

    std::string logoExpected = companyInsertedEdited.logo();
    EXPECT_EQ(logoString, logoExpected);

    // Save data as file
    EXPECT_NO_THROW(StdBinary::toBinary(logoToWritePath.c_str(), logoExpected));

    std::string logoEditString;
    EXPECT_NO_THROW(logoEditString = StdBinary::toStdString(logoEditPath.c_str()));
    companyToSend.set_logo(logoEditString);
    companyToSend.set_uid(result.uid());

    status = client.EditCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());

    status = client.QueryCompanyByUid(companyUid, companyInsertedEdited);
    EXPECT_TRUE(status.ok());
    std::string logoEditedExpected = companyInsertedEdited.logo();
    EXPECT_EQ(logoEditString, logoEditedExpected);
    // Save data as file
    EXPECT_NO_THROW(StdBinary::toBinary(logoEditToWritePath.c_str(), logoEditedExpected));

    // Delete
    status = client.DeleteCompany(companyToSend, result);
    EXPECT_TRUE(status.ok());
}


// qint64 QDateTime::toSecsSinceEpoch() const

// QDateTime QDateTime::fromSecsSinceEpoch(qint64 secs, const QTimeZone &timeZone)
/*
#include <QImage>
#include <QByteArray>
#include <QLabel>
#include <QPixmap>
#include <fstream>
#include <string>
#include <sstream>

// ... inside a class or function

// 1. Read image data to std::string (assuming 'image.png' exists)
std::ifstream fin("image.png", std::ios::binary);
std::stringstream ss;
ss << fin.rdbuf();
std::string imageData = ss.str();

// 2. Convert to QByteArray
QByteArray byteArray = QByteArray::fromStdString(imageData);

// 3. Load QImage
QImage image;
image.loadFromData(byteArray);

// 4. Display in QLabel
QLabel* imageLabel = new QLabel(this);
QPixmap pixmap = QPixmap::fromImage(image);
imageLabel->setPixmap(pixmap);
imageLabel->show();

QImage myImage("path/to/my/image.png"); // Load or create your QImage
QVariant imageVariant = QVariant::fromValue(myImage);
*/
