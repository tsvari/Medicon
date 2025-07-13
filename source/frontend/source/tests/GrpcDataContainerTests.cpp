#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TypeToStringFormatter.h"
#include "include_frontend_util.h"
#include "TestSharedUtility.h"


#include <QDateTime>
#include "GrpcDataContainer.hpp"
//using FrontConverter::to_str;
//using CommonUtil::sqlRowOffset;

using ::testing::ElementsAre;
using ::testing::Pointwise;


namespace {
void compareObjects (const GprcTestDataObject & left, const GprcTestDataObject & right) {
    EXPECT_EQ(left.name(), right.name());
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.date(), DataInfo::Date));
    EXPECT_EQ(left.height(), right.height());
    EXPECT_EQ(left.salary(), right.salary());
    EXPECT_EQ(left.married(), right.married());
}

// Company object has 5 properties
const int PROPERTIES = 5;
}

TEST(GrpcDataContainerTests, GprcTestDataObjectTest)
{
    QDateTime current = QDateTime::currentDateTime();

    GprcTestDataObject company1;

    company1.set_name("Givi");
    company1.set_date(current.toSecsSinceEpoch());
    company1.set_height(168);
    company1.set_salary(12.15);
    company1.set_married(false);

    GprcTestDataObject company2;

    company2.set_name("Keto");
    company2.set_date(current.toSecsSinceEpoch());
    company2.set_height(164);
    company2.set_salary(30.557);
    company2.set_married(true);

    std::vector<GprcTestDataObject> objects;
    objects.push_back(company1);
    objects.push_back(company2);

    GrpcDataContainer<GprcTestDataObject> container(std::move(objects));

    container.addProperty("Name", DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
    container.addProperty("Date", DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
    container.addProperty("Height", DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
    container.addProperty("Salary", DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
    container.addProperty("Married", DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
    container.initialize();

    // vector shouls be empty after using
    EXPECT_TRUE(objects.empty());

    GprcTestDataObject CompanyExpect1 = container.object(0);
    GprcTestDataObject CompanyExpect2 = container.object(1);

    // Check objects
    compareObjects(company1, CompanyExpect1);
    compareObjects(company2, CompanyExpect2);

    // Check properies count
    EXPECT_EQ(container.propertyCount(), PROPERTIES);

    std::vector<GrpcVariantGet> company1Var = {
        company1.name(),
        company1.date(),
        company1.height(),
        company1.salary(),
        company1.married()
    };
    std::vector<GrpcVariantGet> actual1 = {
        container.nativeData(0, 0),
        container.nativeData(0, 1),
        container.nativeData(0, 2),
        container.nativeData(0, 3),
        container.nativeData(0, 4)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(company1Var, actual1));
    ///////////////////////////////////////////////////////////
    /// \brief
    ///

    std::vector<GrpcVariantGet> company2Var = {
       company2.name(),
       company2.date(),
       company2.height(),
       company2.salary(),
       company2.married()
    };
    std::vector<GrpcVariantGet> actual2 = {
        container.nativeData(1, 0),
        container.nativeData(1, 1),
        container.nativeData(1, 2),
        container.nativeData(1, 3),
        container.nativeData(1, 4)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(company2Var, actual2));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> company1QVar = {
        FrontConverter::to_qvariant_get(company1.name()),
        FrontConverter::to_qvariant_get(company1.date()),
        FrontConverter::to_qvariant_get(company1.height()),
        FrontConverter::to_qvariant_get(company1.salary()),
        FrontConverter::to_qvariant_get(company1.married()),
    };
    QList<QVariant> actualQVar1 = {
        container.data(0, 0),
        container.data(0, 1),
        container.data(0, 2),
        container.data(0, 3),
        container.data(0, 4)
    };
    //=========================================
    EXPECT_TRUE(compareQVariantList(company1QVar, actualQVar1));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> company2QVar = {
        FrontConverter::to_qvariant_get(company2.name()),
        FrontConverter::to_qvariant_get(company2.date()),
        FrontConverter::to_qvariant_get(company2.height()),
        FrontConverter::to_qvariant_get(company2.salary()),
        FrontConverter::to_qvariant_get(company2.married())
    };

    QList<QVariant> actualQVar2 = {
        container.data(1, 0),
        container.data(1, 1),
        container.data(1, 2),
        container.data(1, 3),
        container.data(1, 4)
    };
    EXPECT_TRUE(compareQVariantList(company2QVar, actualQVar2));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    std::string newName = "Vakho";
    int64_t newDate = 12345;
    int32_t newHeight = 180;
    double newSalary = 56.78;
    bool newMarried = true;

    GprcTestDataObject actualObject;

    QVariant newVariant = QString::fromStdString(newName);
    GrpcVariantSet newGrpcVariant = newName;
    container.setData(0,0, newGrpcVariant);
    container.setData(1,0, newVariant);
    actualObject.set_name(newName);

    newVariant = newDate;
    newGrpcVariant = newDate;
    container.setData(0,1, newGrpcVariant);
    container.setData(1,1, newVariant);
    actualObject.set_date(newDate);

    newVariant = newHeight;
    newGrpcVariant = newHeight;
    container.setData(0,2, newGrpcVariant);
    container.setData(1,2, newVariant);
    actualObject.set_height(newHeight);

    newVariant = newSalary;
    newGrpcVariant = newSalary;
    container.setData(0,3, newGrpcVariant);
    container.setData(1,3, newVariant);
    actualObject.set_salary(newSalary);

    newVariant = newMarried;
    newGrpcVariant = newMarried;
    container.setData(0,4, newGrpcVariant);
    container.setData(1,4, newVariant);
    actualObject.set_married(newMarried);

    compareObjects(actualObject, container.object(0));
    compareObjects(actualObject, container.object(1));

    // second
    container.insert(1, company1);
    // last/forth
    container.addNew(company2);

    EXPECT_EQ(container.count(), 4);

    compareObjects(company1, container.object(1));
    compareObjects(company2, container.object(3));

    container.remove(2);
    EXPECT_EQ(container.count(), 3);

    container.remove(0);
    EXPECT_EQ(container.count(), 2);

    compareObjects(company1, container.object(0));
    compareObjects(company2, container.object(1));

}



