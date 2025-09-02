#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "company_client.hpp"
#include "TypeToStringFormatter.h"
#include "JsonParameterFormatter.h"
#include "include_frontend_util.h"
#include "TestSharedUtility.h"

#include "front_common.h"
#include <functional>

#include <QDateTime>
#include "GrpcObjectTableModel.h"
//using FrontConverter::to_str;
//using CommonUtil::sqlRowOffset;

using ::testing::ElementsAre;
using ::testing::Pointwise;


namespace {

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
}

// Company object has 9 properties
const int PROPERTIES = 9;
}

TEST(GrpcObjectTableModelTests, GrpcDataContainerTests)
{
    //CompanyEditorClient client(grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));
    QDateTime current = QDateTime::currentDateTime();

    Company company1;
    company1.set_uid("1");
    company1.set_server_uid(server_uid);   
    company1.set_company_type(0);
    company1.set_name("Givi");
    company1.set_address("134 George St, New Brunswick, NJ 08901");
    company1.set_reg_date(current.toSecsSinceEpoch());
    company1.set_joint_date(current.toSecsSinceEpoch());
    company1.set_license("54321");
    company1.set_logo("file.png");


    Company company2;
    company2.set_uid("2");
    company2.set_server_uid(server_uid);
    company2.set_company_type(0);
    company2.set_name("Keto");
    company2.set_address("6063 Rousvelt Blvd, Philadelphia, PA 19149");
    company2.set_reg_date(current.toSecsSinceEpoch());
    company2.set_joint_date(current.toSecsSinceEpoch());
    company2.set_license("12345");
    company1.set_logo("file.jpeg");

    std::vector<Company> objects;
    objects.push_back(company1);
    objects.push_back(company2);

    GrpcDataContainer<Company> container(std::move(objects));
    container.addProperty("", DataInfo::String, &Company::set_uid, &Company::uid);
    container.addProperty("", DataInfo::Int, &Company::set_server_uid, &Company::server_uid);
    container.addProperty("Company type", DataInfo::Int, &Company::set_company_type, &Company::company_type);
    container.addProperty("Name", DataInfo::String, &Company::set_name, &Company::name);
    container.addProperty("Address", DataInfo::String, &Company::set_address, &Company::address);
    container.addProperty("Reg. Date", DataInfo::Date, &Company::set_reg_date, &Company::reg_date);
    container.addProperty("Join Date", DataInfo::Date, &Company::set_joint_date, &Company::joint_date);
    container.addProperty("License", DataInfo::String, &Company::set_license, &Company::license);
    container.addProperty("Logo", DataInfo::String, &Company::set_logo, &Company::logo);
    container.initialize();

    Company CompanyExpect1 = container.object(0);
    Company CompanyExpect2 = container.object(1);

    // Check objects
    compareObjects(company1, CompanyExpect1);
    compareObjects(company2, CompanyExpect2);

    // Check properies count
    EXPECT_EQ(container.propertyCount(), PROPERTIES);

    std::vector<GrpcVariantGet> company1Var = {
        company1.uid(),
        company1.server_uid(),
        company1.company_type(),
        company1.name(),
        company1.address(),
        company1.reg_date(),
        company1.joint_date(),
        company1.license(),
        company1.logo()
    };
    std::vector<GrpcVariantGet> actual1 = {
        container.nativeData(0, 0),
        container.nativeData(0, 1),
        container.nativeData(0, 2),
        container.nativeData(0, 3),
        container.nativeData(0, 4),
        container.nativeData(0, 5),
        container.nativeData(0, 6),
        container.nativeData(0, 7),
        container.nativeData(0, 8)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(company1Var, actual1));
    ///////////////////////////////////////////////////////////
    /// \brief
    ///

    std::vector<GrpcVariantGet> company2Var = {
        company2.uid(),
        company2.server_uid(),
        company2.company_type(),
        company2.name(),
        company2.address(),
        company2.reg_date(),
        company2.joint_date(),
        company2.license(),
        company2.logo()
    };
    std::vector<GrpcVariantGet> actual2 = {
        container.nativeData(1, 0),
        container.nativeData(1, 1),
        container.nativeData(1, 2),
        container.nativeData(1, 3),
        container.nativeData(1, 4),
        container.nativeData(1, 5),
        container.nativeData(1, 6),
        container.nativeData(1, 7),
        container.nativeData(1, 8)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(company2Var, actual2));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> company1QVar = {
        FrontConverter::to_qvariant_get(company1.uid()),
        FrontConverter::to_qvariant_get(company1.server_uid()),
        FrontConverter::to_qvariant_get(company1.company_type()),
        FrontConverter::to_qvariant_get(company1.name()),
        FrontConverter::to_qvariant_get(company1.address()),
        FrontConverter::to_qvariant_get(company1.reg_date()),
        FrontConverter::to_qvariant_get(company1.joint_date()),
        FrontConverter::to_qvariant_get(company1.license()),
        FrontConverter::to_qvariant_get(company1.logo())
    };
    QList<QVariant> actualQVar1 = {
        container.data(0, 0),
        container.data(0, 1),
        container.data(0, 2),
        container.data(0, 3),
        container.data(0, 4),
        container.data(0, 5),
        container.data(0, 6),
        container.data(0, 7),
        container.data(0, 8)
    };
    //=========================================
    EXPECT_TRUE(compareQVariantList(company1QVar, actualQVar1));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> company2QVar = {
            FrontConverter::to_qvariant_get(company2.uid()),
            FrontConverter::to_qvariant_get(company2.server_uid()),
            FrontConverter::to_qvariant_get(company2.company_type()),
            FrontConverter::to_qvariant_get(company2.name()),
            FrontConverter::to_qvariant_get(company2.address()),
            FrontConverter::to_qvariant_get(company2.reg_date()),
            FrontConverter::to_qvariant_get(company2.joint_date()),
            FrontConverter::to_qvariant_get(company2.license()),
            FrontConverter::to_qvariant_get(company2.logo())
    };

    QList<QVariant> actualQVar2 = {
        container.data(1, 0),
        container.data(1, 1),
        container.data(1, 2),
        container.data(1, 3),
        container.data(1, 4),
        container.data(1, 5),
        container.data(1, 6),
        container.data(1, 7),
        container.data(1, 8)
    };
    EXPECT_TRUE(compareQVariantList(company2QVar, actualQVar2));
    //////////////////////////////////////////////////////
    /// \brief
    ///
}

TEST(GrpcObjectTableModelTests, ModelTest)
{

}

