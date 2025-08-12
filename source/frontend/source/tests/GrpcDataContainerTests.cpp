#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TypeToStringFormatter.h"
#include "TestSharedUtility.h"
#include "GrpcDataContainer.hpp"

#include <QDateTime>

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

    GprcTestDataObject obj1;
    obj1.set_name("Givi");
    obj1.set_date(current.toSecsSinceEpoch());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);

    GprcTestDataObject obj2;
    obj2.set_name("Keto");
    obj2.set_date(current.toSecsSinceEpoch());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);

    std::vector<GprcTestDataObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);

    GrpcDataContainer<GprcTestDataObject> container(std::move(objects));

    container.addProperty("Name", DataInfo::String, &GprcTestDataObject::set_name, &GprcTestDataObject::name);
    container.addProperty("Date", DataInfo::Date, &GprcTestDataObject::set_date, &GprcTestDataObject::date);
    container.addProperty("Height", DataInfo::Int, &GprcTestDataObject::set_height, &GprcTestDataObject::height);
    container.addProperty("Salary", DataInfo::Double, &GprcTestDataObject::set_salary, &GprcTestDataObject::salary);
    container.addProperty("Married", DataInfo::Bool, &GprcTestDataObject::set_married, &GprcTestDataObject::married);
    container.initialize();

    // vector shouls be empty after using
    //EXPECT_TRUE(objects.empty());

    QVariant variantObject = container.variantObject(0);
    EXPECT_TRUE(variantObject.isValid());
    GprcTestDataObject expectedRealObject = variantObject.value<GprcTestDataObject>();
    compareObjects(obj1, expectedRealObject);

    GprcTestDataObject CompanyExpect1 = container.object(0);
    GprcTestDataObject CompanyExpect2 = container.object(1);

    // Check objects
    compareObjects(obj1, CompanyExpect1);
    compareObjects(obj2, CompanyExpect2);

    // Check properies count
    EXPECT_EQ(container.propertyCount(), PROPERTIES);

    std::vector<GrpcVariantGet> obj1Var = {
        obj1.name(),
        obj1.date(),
        obj1.height(),
        obj1.salary(),
        obj1.married()
    };
    std::vector<GrpcVariantGet> actual1 = {
        container.nativeData(0, 0),
        container.nativeData(0, 1),
        container.nativeData(0, 2),
        container.nativeData(0, 3),
        container.nativeData(0, 4)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(obj1Var, actual1));
    ///////////////////////////////////////////////////////////
    /// \brief
    ///

    std::vector<GrpcVariantGet> obj2Var = {
       obj2.name(),
       obj2.date(),
       obj2.height(),
       obj2.salary(),
       obj2.married()
    };
    std::vector<GrpcVariantGet> actual2 = {
        container.nativeData(1, 0),
        container.nativeData(1, 1),
        container.nativeData(1, 2),
        container.nativeData(1, 3),
        container.nativeData(1, 4)
    };
    //=========================================
    EXPECT_TRUE(loose_vector_compare(obj2Var, actual2));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> obj1QVar = {
        FrontConverter::to_qvariant_get(obj1.name()),
        FrontConverter::to_qvariant_get(obj1.date()),
        FrontConverter::to_qvariant_get(obj1.height()),
        FrontConverter::to_qvariant_get(obj1.salary()),
        FrontConverter::to_qvariant_get(obj1.married()),
    };
    QList<QVariant> actualQVar1 = {
        container.data(0, 0),
        container.data(0, 1),
        container.data(0, 2),
        container.data(0, 3),
        container.data(0, 4)
    };
    //=========================================
    EXPECT_TRUE(compareQVariantList(obj1QVar, actualQVar1));
    //////////////////////////////////////////////////////
    /// \brief
    ///

    QList<QVariant> obj2QVar = {
        FrontConverter::to_qvariant_get(obj2.name()),
        FrontConverter::to_qvariant_get(obj2.date()),
        FrontConverter::to_qvariant_get(obj2.height()),
        FrontConverter::to_qvariant_get(obj2.salary()),
        FrontConverter::to_qvariant_get(obj2.married())
    };

    QList<QVariant> actualQVar2 = {
        container.data(1, 0),
        container.data(1, 1),
        container.data(1, 2),
        container.data(1, 3),
        container.data(1, 4)
    };
    EXPECT_TRUE(compareQVariantList(obj2QVar, actualQVar2));
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
    container.insert(1, obj1);
    EXPECT_TRUE(container.count() == 3);
    // CHeck inserted object data
    EXPECT_EQ(FrontConverter::to_str(container.data(1, 0)), obj1.name());
    EXPECT_EQ(container.data(1, 1).toLongLong(), obj1.date());
    EXPECT_EQ(container.data(1, 2).toInt(), obj1.height());
    EXPECT_EQ(container.data(1, 3).toDouble(), obj1.salary());
    EXPECT_EQ(container.data(1, 4).toBool(), obj1.married());

    EXPECT_EQ("Vakho", container.data(0, 0).toString());
    EXPECT_EQ("Givi", container.data(1, 0).toString());
    EXPECT_EQ("Vakho", container.data(2, 0).toString());

    // last/forth
    container.addNew(obj2);
    EXPECT_TRUE(container.count() == 4);
    // CHeck added object data
    EXPECT_EQ(FrontConverter::to_str(container.data(3, 0)), obj2.name());
    EXPECT_EQ(container.data(3, 1).toLongLong(), obj2.date());
    EXPECT_EQ(container.data(3, 2).toInt(), obj2.height());
    EXPECT_EQ(container.data(3, 3).toDouble(), obj2.salary());
    EXPECT_EQ(container.data(3, 4).toBool(), obj2.married());

    EXPECT_EQ("Vakho", container.data(0, 0).toString());
    EXPECT_EQ("Givi", container.data(1, 0).toString());
    EXPECT_EQ("Vakho", container.data(2, 0).toString());
    EXPECT_EQ("Keto", container.data(3, 0).toString());

    compareObjects(obj1, container.object(1));
    compareObjects(obj2, container.object(3));

    container.remove(2);
    EXPECT_EQ(container.count(), 3);

    container.remove(0);
    EXPECT_EQ(container.count(), 2);

    compareObjects(obj1, container.object(0));
    compareObjects(obj2, container.object(1));

    EXPECT_EQ("Givi", container.data(0, 0).toString());
    EXPECT_EQ("Keto", container.data(1, 0).toString());

    GprcTestDataObject newObj;
    newObj.set_name("Vakho");
    newObj.set_date(current.toSecsSinceEpoch());
    newObj.set_height(175);
    newObj.set_salary(145.123);
    newObj.set_married(true);

    container.update(1, newObj);
    compareObjects(newObj, container.object(1));
    EXPECT_EQ("Givi", container.data(0, 0).toString());
    EXPECT_EQ("Vakho", container.data(1, 0).toString());
}


