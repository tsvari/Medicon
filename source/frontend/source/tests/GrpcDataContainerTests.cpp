#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TypeToStringFormatter.h"
#include "TestSharedUtility.h"
#include "GrpcDataContainer.hpp"

#include <QDateTime>

using ::testing::ElementsAre;
using ::testing::Pointwise;

namespace {
void compareObjects (const MasterObject & left, const MasterObject & right) {
    EXPECT_EQ(left.name(), right.name());
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.date(), DataInfo::Date));
    EXPECT_EQ(left.height(), right.height());
    EXPECT_EQ(left.salary(), right.salary());
    EXPECT_EQ(left.married(), right.married());
}

// Company object has 5 properties
const int PROPERTIES = 5;
}

TEST(GrpcDataContainerTests, MasterObjectTest)
{
    QDateTime current = QDateTime::currentDateTime();

    MasterObject obj1;
    obj1.set_name("Givi");
    obj1.set_date(current.toSecsSinceEpoch());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);

    MasterObject obj2;
    obj2.set_name("Keto");
    obj2.set_date(current.toSecsSinceEpoch());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);

    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);

    GrpcDataContainer<MasterObject> container(std::move(objects));

    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    container.initialize();

    // vector shouls be empty after using
    //EXPECT_TRUE(objects.empty());

    QVariant variantObject = container.variantObject(0);
    EXPECT_TRUE(variantObject.isValid());
    MasterObject expectedRealObject = variantObject.value<MasterObject>();
    compareObjects(obj1, expectedRealObject);

    MasterObject CompanyExpect1 = container.object(0);
    MasterObject CompanyExpect2 = container.object(1);

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

    MasterObject actualObject;

    QVariant newVariant = QString::fromStdString(newName);
    GrpcVariantSet newGrpcVariant = newName;
    container.setData(0,0, newGrpcVariant);
    container.setData(1,0, newVariant);
    actualObject.set_name(newName);

    newVariant.fromValue<int64_t>(newDate);
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

    MasterObject newObj;
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

// ============================================================================
// Additional Tests for Improved GrpcDataContainer
// ============================================================================

TEST(GrpcDataContainerTests, DefaultConstructorTest)
{
    GrpcDataContainer<MasterObject> container;
    
    EXPECT_EQ(container.count(), 0);
    EXPECT_EQ(container.propertyCount(), 0);
}

TEST(GrpcDataContainerTests, ConstMethodsTest)
{
    MasterObject obj;
    obj.set_name("Test");
    obj.set_height(170);
    obj.set_salary(50000.0);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.initialize();
    
    // Test const methods
    const GrpcDataContainer<MasterObject>& constContainer = container;
    
    EXPECT_EQ(constContainer.count(), 1);
    EXPECT_EQ(constContainer.propertyCount(), 3);
    EXPECT_EQ(constContainer.object(0).name(), "Test");
    EXPECT_EQ(constContainer.data(0, 1).toInt(), 170);
    EXPECT_DOUBLE_EQ(constContainer.data(0, 2).toDouble(), 50000.0);
}

TEST(GrpcDataContainerTests, MoveConstructorTest)
{
    MasterObject obj;
    obj.set_name("Test");
    obj.set_height(175);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container1(std::move(objects));
    container1.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container1.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container1.initialize();
    
    // Move constructor
    GrpcDataContainer<MasterObject> container2(std::move(container1));
    
    EXPECT_EQ(container2.count(), 1);
    EXPECT_EQ(container2.propertyCount(), 2);
    EXPECT_EQ(container2.object(0).name(), "Test");
}

TEST(GrpcDataContainerTests, InsertAtBeginningTest)
{
    MasterObject obj1, obj2, obj3;
    obj1.set_name("First");
    obj2.set_name("Second");
    obj3.set_name("Third");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    // Insert at beginning
    container.insert(0, obj3);
    
    EXPECT_EQ(container.count(), 3);
    EXPECT_EQ(container.data(0, 0).toString(), "Third");
    EXPECT_EQ(container.data(1, 0).toString(), "First");
    EXPECT_EQ(container.data(2, 0).toString(), "Second");
}

TEST(GrpcDataContainerTests, InsertAtEndTest)
{
    MasterObject obj1, obj2, obj3;
    obj1.set_name("First");
    obj2.set_name("Second");
    obj3.set_name("Third");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    // Insert at end
    container.insert(container.count(), obj3);
    
    EXPECT_EQ(container.count(), 3);
    EXPECT_EQ(container.data(0, 0).toString(), "First");
    EXPECT_EQ(container.data(1, 0).toString(), "Second");
    EXPECT_EQ(container.data(2, 0).toString(), "Third");
}

TEST(GrpcDataContainerTests, MultipleRemovalsTest)
{
    std::vector<MasterObject> objects;
    for (int i = 0; i < 5; ++i) {
        MasterObject obj;
        obj.set_name("Object" + std::to_string(i));
        obj.set_height(160 + i);
        objects.push_back(obj);
    }
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.initialize();
    
    EXPECT_EQ(container.count(), 5);
    
    // Remove middle element
    container.remove(2);
    EXPECT_EQ(container.count(), 4);
    EXPECT_EQ(container.data(2, 0).toString(), "Object3");
    
    // Remove first element
    container.remove(0);
    EXPECT_EQ(container.count(), 3);
    EXPECT_EQ(container.data(0, 0).toString(), "Object1");
    
    // Remove last element
    container.remove(2);
    EXPECT_EQ(container.count(), 2);
    EXPECT_EQ(container.data(0, 0).toString(), "Object1");
    EXPECT_EQ(container.data(1, 0).toString(), "Object3");
}

TEST(GrpcDataContainerTests, UpdateMultipleTimesTest)
{
    MasterObject obj;
    obj.set_name("Original");
    obj.set_height(170);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.initialize();
    
    // First update
    MasterObject updated1;
    updated1.set_name("Updated1");
    updated1.set_height(175);
    container.update(0, updated1);
    
    EXPECT_EQ(container.data(0, 0).toString(), "Updated1");
    EXPECT_EQ(container.data(0, 1).toInt(), 175);
    
    // Second update
    MasterObject updated2;
    updated2.set_name("Updated2");
    updated2.set_height(180);
    container.update(0, updated2);
    
    EXPECT_EQ(container.data(0, 0).toString(), "Updated2");
    EXPECT_EQ(container.data(0, 1).toInt(), 180);
}

TEST(GrpcDataContainerTests, HorizontalHeaderDataTest)
{
    GrpcDataContainer<MasterObject> container;
    
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    
    EXPECT_EQ(container.horizontalHeaderData(0).toString(), "Name");
    EXPECT_EQ(container.horizontalHeaderData(1).toString(), "Height");
    EXPECT_EQ(container.horizontalHeaderData(2).toString(), "Salary");
    EXPECT_EQ(container.horizontalHeaderData(3).toString(), "Married");
}

TEST(GrpcDataContainerTests, DataTypeTest)
{
    GrpcDataContainer<MasterObject> container;
    
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    container.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    
    EXPECT_EQ(container.dataType(0), DataInfo::String);
    EXPECT_EQ(container.dataType(1), DataInfo::Int);
    EXPECT_EQ(container.dataType(2), DataInfo::Double);
    EXPECT_EQ(container.dataType(3), DataInfo::Bool);
    EXPECT_EQ(container.dataType(4), DataInfo::Date);
}

TEST(GrpcDataContainerTests, SetDataWithQVariantTypesTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    container.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    container.initialize();
    
    // Test string
    container.setData(0, 0, QVariant(QString("TestName")));
    EXPECT_EQ(container.data(0, 0).toString(), "TestName");
    
    // Test int
    container.setData(0, 1, QVariant(185));
    EXPECT_EQ(container.data(0, 1).toInt(), 185);
    
    // Test double
    container.setData(0, 2, QVariant(99999.99));
    EXPECT_DOUBLE_EQ(container.data(0, 2).toDouble(), 99999.99);
    
    // Test bool
    container.setData(0, 3, QVariant(true));
    EXPECT_TRUE(container.data(0, 3).toBool());
    
    // Test date (int64)
    container.setData(0, 4, QVariant(qint64(1704110400)));
    EXPECT_EQ(container.data(0, 4).toLongLong(), 1704110400);
}

TEST(GrpcDataContainerTests, VariantObjectTest)
{
    MasterObject obj;
    obj.set_name("TestObject");
    obj.set_height(172);
    obj.set_salary(55000.50);
    obj.set_married(true);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    QVariant variant = container.variantObject(0);
    EXPECT_TRUE(variant.isValid());
    EXPECT_TRUE(variant.canConvert<MasterObject>());
    
    MasterObject extracted = variant.value<MasterObject>();
    EXPECT_EQ(extracted.name(), "TestObject");
    EXPECT_EQ(extracted.height(), 172);
    EXPECT_DOUBLE_EQ(extracted.salary(), 55000.50);
    EXPECT_TRUE(extracted.married());
}

TEST(GrpcDataContainerTests, InsertObjectWithQVariantTest)
{
    MasterObject obj1, obj2;
    obj1.set_name("First");
    obj2.set_name("Second");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    QVariant variant = QVariant::fromValue(obj2);
    container.insertObject(1, variant);
    
    EXPECT_EQ(container.count(), 2);
    EXPECT_EQ(container.data(0, 0).toString(), "First");
    EXPECT_EQ(container.data(1, 0).toString(), "Second");
}

TEST(GrpcDataContainerTests, UpdateObjectWithQVariantTest)
{
    MasterObject obj1, obj2;
    obj1.set_name("Original");
    obj2.set_name("Updated");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    QVariant variant = QVariant::fromValue(obj2);
    container.updateObject(0, variant);
    
    EXPECT_EQ(container.count(), 1);
    EXPECT_EQ(container.data(0, 0).toString(), "Updated");
}

TEST(GrpcDataContainerTests, AddNewObjectWithQVariantTest)
{
    MasterObject obj;
    obj.set_name("New");
    
    GrpcDataContainer<MasterObject> container;
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    QVariant variant = QVariant::fromValue(obj);
    container.addNewObject(variant);
    
    EXPECT_EQ(container.count(), 1);
    EXPECT_EQ(container.data(0, 0).toString(), "New");
}

TEST(GrpcDataContainerTests, DeleteObjectTest)
{
    MasterObject obj1, obj2, obj3;
    obj1.set_name("First");
    obj2.set_name("Second");
    obj3.set_name("Third");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);
    objects.push_back(obj3);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.initialize();
    
    container.deleteObject(1);
    
    EXPECT_EQ(container.count(), 2);
    EXPECT_EQ(container.data(0, 0).toString(), "First");
    EXPECT_EQ(container.data(1, 0).toString(), "Third");
}

TEST(GrpcDataContainerTests, AllDataTypesTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    MasterObject obj;
    obj.set_uid(42);
    obj.set_name("Complete");
    obj.set_date(current.toSecsSinceEpoch());
    obj.set_time(120000);
    obj.set_date_time(current.toSecsSinceEpoch());
    obj.set_date_time_no_sec(current.toSecsSinceEpoch());
    obj.set_height(180);
    obj.set_salary(75000.75);
    obj.set_married(true);
    obj.set_level(3);
    obj.set_level_name("Senior");
    obj.set_married_name("Yes");
    obj.set_image("/path/to/image.png");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcDataContainer<MasterObject> container(std::move(objects));
    container.addProperty("Uid", DataInfo::Int, &MasterObject::set_uid, &MasterObject::uid);
    container.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    container.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    container.addProperty("Time", DataInfo::Int64, &MasterObject::set_time, &MasterObject::time);
    container.addProperty("DateTime", DataInfo::DateTime, &MasterObject::set_date_time, &MasterObject::date_time);
    container.addProperty("DateTimeNoSec", DataInfo::DateTimeNoSec, &MasterObject::set_date_time_no_sec, &MasterObject::date_time_no_sec);
    container.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    container.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    container.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    container.addProperty("Level", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);
    container.addProperty("LevelName", DataInfo::String, &MasterObject::set_level_name, &MasterObject::level_name);
    container.addProperty("MarriedName", DataInfo::String, &MasterObject::set_married_name, &MasterObject::married_name);
    container.addProperty("Image", DataInfo::String, &MasterObject::set_image, &MasterObject::image);
    container.initialize();
    
    EXPECT_EQ(container.propertyCount(), 13);
    EXPECT_EQ(container.data(0, 0).toInt(), 42);
    EXPECT_EQ(container.data(0, 1).toString(), "Complete");
    EXPECT_EQ(container.data(0, 2).toLongLong(), current.toSecsSinceEpoch());
    EXPECT_EQ(container.data(0, 3).toLongLong(), 120000);
    EXPECT_EQ(container.data(0, 6).toInt(), 180);
    EXPECT_DOUBLE_EQ(container.data(0, 7).toDouble(), 75000.75);
    EXPECT_TRUE(container.data(0, 8).toBool());
    EXPECT_EQ(container.data(0, 9).toInt(), 3);
    EXPECT_EQ(container.data(0, 10).toString(), "Senior");
    EXPECT_EQ(container.data(0, 11).toString(), "Yes");
    EXPECT_EQ(container.data(0, 12).toString(), "/path/to/image.png");
}

TEST(GrpcDataContainerTests, SlaveObjectTest)
{
    SlaveObject obj1(1, 100, "+1234567890");
    SlaveObject obj2(2, 100, "+0987654321");
    
    std::vector<SlaveObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);
    
    GrpcDataContainer<SlaveObject> container(std::move(objects));
    container.addProperty("Uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
    container.addProperty("LinkUid", DataInfo::Int, &SlaveObject::set_link_uid, &SlaveObject::link_uid);
    container.addProperty("Phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    container.initialize();
    
    EXPECT_EQ(container.count(), 2);
    EXPECT_EQ(container.propertyCount(), 3);
    
    EXPECT_EQ(container.data(0, 0).toInt(), 1);
    EXPECT_EQ(container.data(0, 1).toInt(), 100);
    EXPECT_EQ(container.data(0, 2).toString(), "+1234567890");
    
    EXPECT_EQ(container.data(1, 0).toInt(), 2);
    EXPECT_EQ(container.data(1, 1).toInt(), 100);
    EXPECT_EQ(container.data(1, 2).toString(), "+0987654321");
    
    // Update slave object
    SlaveObject newObj(3, 200, "+1111111111");
    container.update(0, newObj);
    
    EXPECT_EQ(container.data(0, 0).toInt(), 3);
    EXPECT_EQ(container.data(0, 1).toInt(), 200);
    EXPECT_EQ(container.data(0, 2).toString(), "+1111111111");
}
