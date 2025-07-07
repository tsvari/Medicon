#include "gtest/gtest.h"
#include "company_client.hpp"
#include "TypeToStringFormatter.h"
#include "JsonParameterFormatter.h"
#include "include_frontend_util.h"

#include "front_common.h"
#include <functional>

#include <QDateTime>
#include "GrpcObjectTableModel.h"
//using FrontConverter::to_str;
//using CommonUtil::sqlRowOffset;

TEST(GrpcObjectTableModelTests, GrpcDataControlerTests)
{
    CompanyEditorClient client(grpc::CreateChannel(channelAddress, grpc::InsecureChannelCredentials()));
    QDateTime current = QDateTime::currentDateTime();

    Company company1;
    company1.set_server_uid(server_uid);
    company1.set_company_type(0);
    company1.set_address("134 George St, New Brunswick, NJ 08901");
    company1.set_reg_date(current.toSecsSinceEpoch());
    company1.set_joint_date(current.toSecsSinceEpoch());
    company1.set_license("54321");
    company1.set_name("Givi");
    company1.set_logo("file.png");


    Company company2;
    company2.set_server_uid(server_uid);
    company2.set_company_type(0);
    company2.set_address("6063 Rousvelt Blvd, Philadelphia, PA 19149");
    company2.set_reg_date(current.toSecsSinceEpoch());
    company2.set_joint_date(current.toSecsSinceEpoch());
    company2.set_license("12345");
    company2.set_name("Keto");
    company1.set_logo("file.jpeg");

    company2.name();
    company2.server_uid();
    company2.joint_date();

    std::vector<Company> objects;
    objects.push_back(company1);
    objects.push_back(company2);


    GrpcDataController<Company> controller(std::move(objects));
    controller.addProperty("", DataInfo::Int, &Company::set_server_uid, &Company::server_uid);
    controller.addProperty("Company type", DataInfo::Int, &Company::set_company_type, &Company::company_type);
    controller.addProperty("Name", DataInfo::String, &Company::set_name, &Company::name);
    controller.addProperty("Address", DataInfo::String, &Company::set_address, &Company::address);
    controller.addProperty("Reg. Date", DataInfo::Date, &Company::set_reg_date, &Company::reg_date);
    controller.addProperty("Join Date", DataInfo::Date, &Company::set_joint_date, &Company::joint_date);
    controller.addProperty("License", DataInfo::String, &Company::set_license, &Company::license);
    controller.addProperty("Logo", DataInfo::String, &Company::set_logo, &Company::logo);
    controller.initialize();

    //EXPECT_EQ(controller.)

    std::string oldData = FrontConverter::to_str(controller.data(0,0));
    std::string oldData2 = FrontConverter::to_str(controller.data(1,0));

    //Company & proccesedObject = controller.object(0);
    //Company & proccesedObject2 = controller.object(1);

    controller.setData(0,0, "Vakho");
    controller.setData(1,0, "Vakho");

    //std::string newData = proccesedObject.name();
    //std::string newData2 = proccesedObject2.name();

    std::string newData = FrontConverter::to_str(controller.data(0,0));
    std::string newData2 = FrontConverter::to_str(controller.data(1,0));

    int32_t oldInt = controller.data(0,2).toInt();
    int64_t oldInt64 = controller.data(0,3).toInt();

    controller.setData(0,2, 101);
    controller.setData(0,3, 888888);

    int32_t newInt = controller.data(0,2).toInt();
    int64_t newInt64 = controller.data(0,3).toInt();


    int k  = 0;

}

TEST(GrpcObjectTableModelTests, ModelTest)
{

}

