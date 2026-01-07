#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "TypeToStringFormatter.h"
#include "TestSharedUtility.h"
#include "GrpcObjectWrapper.hpp"

#include <QDateTime>

#include <QCheckBox>
#include <QDateEdit>
#include <QComboBox>
#include <QLineEdit>

namespace {
void compareObjects (const MasterObject & left, const MasterObject & right) {
    EXPECT_EQ(left.uid(), right.uid());
    EXPECT_EQ(left.name(), right.name());
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.date(), DataInfo::Date));
    EXPECT_EQ(left.height(), right.height());
    EXPECT_EQ(left.salary(), right.salary());
    EXPECT_EQ(left.married(), right.married());
}
const int columnCount = 6;
}


TEST(GrpcObjectWrapperTests, AllTests)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("nameEdit", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("dateEdit", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    wrapper.addProperty("heightEdit", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("salaryEdit", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("marriedCheckBox", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    wrapper.addProperty("levelCombo", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);

    QDateTime current = QDateTime::currentDateTime();

    MasterObject obj1;

    obj1.set_uid(1);
    obj1.set_name("Givi");
    obj1.set_date(current.toSecsSinceEpoch());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);
    obj1.set_level(2);
    obj1.set_level_name("Level2");

    wrapper.setObject(QVariant::fromValue<MasterObject>(obj1));

    EXPECT_EQ(wrapper.propertyCount(), columnCount);
    compareObjects(obj1, wrapper.variantObject().value<MasterObject>());

    GrpcVariantSet data = 222;
    wrapper.setData(2, data);
    obj1.set_height(222);
    compareObjects(obj1, wrapper.variantObject().value<MasterObject>());

    wrapper.setData(2, QVariant::fromValue<int32_t>(333));
    obj1.set_height(333);
    compareObjects(obj1, wrapper.variantObject().value<MasterObject>());

    EXPECT_EQ(wrapper.data(0).toString(), QString::fromStdString(obj1.name()));
}

// ============================================================================
// Additional Tests for Improved GrpcObjectWrapper
// ============================================================================

TEST(GrpcObjectWrapperTests, DefaultConstructorTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    
    EXPECT_FALSE(wrapper.hasObject());
    EXPECT_EQ(wrapper.propertyCount(), 0);
    EXPECT_FALSE(wrapper.variantObject().isValid());
}

TEST(GrpcObjectWrapperTests, HasObjectTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    
    EXPECT_FALSE(wrapper.hasObject());
    
    MasterObject obj;
    obj.set_name("Test");
    wrapper.setObject(QVariant::fromValue(obj));
    
    EXPECT_TRUE(wrapper.hasObject());
}

TEST(GrpcObjectWrapperTests, PropertyCountTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    
    EXPECT_EQ(wrapper.propertyCount(), 0);
    
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    EXPECT_EQ(wrapper.propertyCount(), 1);
    
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    EXPECT_EQ(wrapper.propertyCount(), 2);
    
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    EXPECT_EQ(wrapper.propertyCount(), 3);
}

TEST(GrpcObjectWrapperTests, PropertyWidgetNameTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("nameEdit", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("heightSpin", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("salaryEdit", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    
    EXPECT_EQ(wrapper.propertyWidgetName(0).toString(), "nameEdit");
    EXPECT_EQ(wrapper.propertyWidgetName(1).toString(), "heightSpin");
    EXPECT_EQ(wrapper.propertyWidgetName(2).toString(), "salaryEdit");
}

TEST(GrpcObjectWrapperTests, DataTypeTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    wrapper.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    
    EXPECT_EQ(wrapper.dataType(0), DataInfo::String);
    EXPECT_EQ(wrapper.dataType(1), DataInfo::Int);
    EXPECT_EQ(wrapper.dataType(2), DataInfo::Double);
    EXPECT_EQ(wrapper.dataType(3), DataInfo::Bool);
    EXPECT_EQ(wrapper.dataType(4), DataInfo::Date);
}

TEST(GrpcObjectWrapperTests, DataMaskTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name, DataMask::NoMask);
    wrapper.addProperty("Level", DataInfo::Int, &MasterObject::set_level, &MasterObject::level, DataMask::ComboEditMask);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married, DataMask::CheckBoxMask);
    
    EXPECT_EQ(wrapper.dataMask(0), DataMask::NoMask);
    EXPECT_EQ(wrapper.dataMask(1), DataMask::ComboEditMask);
    EXPECT_EQ(wrapper.dataMask(2), DataMask::CheckBoxMask);
}

TEST(GrpcObjectWrapperTests, TrueDataFalseDataTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married, 
                       DataMask::CheckBoxMask, "Yes", "No");
    
    EXPECT_EQ(wrapper.trueData(0).toString(), "Yes");
    EXPECT_EQ(wrapper.falseData(0).toString(), "No");
}

TEST(GrpcObjectWrapperTests, SetObjectAndRetrieveTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    
    MasterObject obj;
    obj.set_name("Alice");
    obj.set_height(165);
    obj.set_salary(60000.50);
    
    wrapper.setObject(QVariant::fromValue(obj));
    
    EXPECT_TRUE(wrapper.hasObject());
    
    QVariant retrieved = wrapper.variantObject();
    EXPECT_TRUE(retrieved.isValid());
    
    MasterObject retrievedObj = retrieved.value<MasterObject>();
    compareObjects(obj, retrievedObj);
}

TEST(GrpcObjectWrapperTests, GetDataAllTypesTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Uid", DataInfo::Int, &MasterObject::set_uid, &MasterObject::uid);
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    
    MasterObject obj;
    obj.set_uid(123);
    obj.set_name("TestUser");
    obj.set_date(current.toSecsSinceEpoch());
    obj.set_height(175);
    obj.set_salary(75000.99);
    obj.set_married(true);
    
    wrapper.setObject(QVariant::fromValue(obj));
    
    EXPECT_EQ(wrapper.data(0).toInt(), 123);
    EXPECT_EQ(wrapper.data(1).toString(), "TestUser");
    EXPECT_EQ(wrapper.data(2).toLongLong(), current.toSecsSinceEpoch());
    EXPECT_EQ(wrapper.data(3).toInt(), 175);
    EXPECT_DOUBLE_EQ(wrapper.data(4).toDouble(), 75000.99);
    EXPECT_TRUE(wrapper.data(5).toBool());
}

TEST(GrpcObjectWrapperTests, SetDataWithGrpcVariantSetTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    
    MasterObject obj;
    wrapper.setObject(QVariant::fromValue(obj));
    
    // Set string
    GrpcVariantSet nameData = std::string("UpdatedName");
    wrapper.setData(0, nameData);
    EXPECT_EQ(wrapper.data(0).toString(), "UpdatedName");
    
    // Set int
    GrpcVariantSet heightData = int32_t(180);
    wrapper.setData(1, heightData);
    EXPECT_EQ(wrapper.data(1).toInt(), 180);
    
    // Set double
    GrpcVariantSet salaryData = 85000.50;
    wrapper.setData(2, salaryData);
    EXPECT_DOUBLE_EQ(wrapper.data(2).toDouble(), 85000.50);
    
    // Set bool
    GrpcVariantSet marriedData = true;
    wrapper.setData(3, marriedData);
    EXPECT_TRUE(wrapper.data(3).toBool());
}

TEST(GrpcObjectWrapperTests, SetDataWithQVariantTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    
    MasterObject obj;
    wrapper.setObject(QVariant::fromValue(obj));
    
    // Set with QVariant
    wrapper.setData(0, QVariant(QString("QVariantName")));
    EXPECT_EQ(wrapper.data(0).toString(), "QVariantName");
    
    wrapper.setData(1, QVariant(190));
    EXPECT_EQ(wrapper.data(1).toInt(), 190);
    
    wrapper.setData(2, QVariant(95000.75));
    EXPECT_DOUBLE_EQ(wrapper.data(2).toDouble(), 95000.75);
    
    wrapper.setData(3, QVariant(false));
    EXPECT_FALSE(wrapper.data(3).toBool());
}

TEST(GrpcObjectWrapperTests, SetDataWithoutObjectTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    
    EXPECT_FALSE(wrapper.hasObject());
    
    // Should not crash when setting data without object
    GrpcVariantSet data = std::string("TestName");
    wrapper.setData(0, data);  // Should do nothing
    
    wrapper.setData(0, QVariant(QString("Test")));  // Should do nothing
    
    // Still no object
    EXPECT_FALSE(wrapper.hasObject());
}

TEST(GrpcObjectWrapperTests, MultipleUpdatesTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    
    MasterObject obj;
    obj.set_name("Original");
    obj.set_height(170);
    wrapper.setObject(QVariant::fromValue(obj));
    
    // First update
    wrapper.setData(0, QVariant(QString("Update1")));
    wrapper.setData(1, QVariant(175));
    EXPECT_EQ(wrapper.data(0).toString(), "Update1");
    EXPECT_EQ(wrapper.data(1).toInt(), 175);
    
    // Second update
    wrapper.setData(0, QVariant(QString("Update2")));
    wrapper.setData(1, QVariant(180));
    EXPECT_EQ(wrapper.data(0).toString(), "Update2");
    EXPECT_EQ(wrapper.data(1).toInt(), 180);
    
    // Third update
    wrapper.setData(0, QVariant(QString("Final")));
    wrapper.setData(1, QVariant(185));
    EXPECT_EQ(wrapper.data(0).toString(), "Final");
    EXPECT_EQ(wrapper.data(1).toInt(), 185);
}

TEST(GrpcObjectWrapperTests, ReplaceObjectTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    
    MasterObject obj1;
    obj1.set_name("First");
    obj1.set_height(170);
    wrapper.setObject(QVariant::fromValue(obj1));
    
    EXPECT_EQ(wrapper.data(0).toString(), "First");
    EXPECT_EQ(wrapper.data(1).toInt(), 170);
    
    // Replace with new object
    MasterObject obj2;
    obj2.set_name("Second");
    obj2.set_height(180);
    wrapper.setObject(QVariant::fromValue(obj2));
    
    EXPECT_EQ(wrapper.data(0).toString(), "Second");
    EXPECT_EQ(wrapper.data(1).toInt(), 180);
}

TEST(GrpcObjectWrapperTests, ConstMethodsTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    
    MasterObject obj;
    obj.set_name("Const Test");
    obj.set_height(172);
    wrapper.setObject(QVariant::fromValue(obj));
    
    // Test const methods
    const GrpcObjectWrapper<MasterObject>& constWrapper = wrapper;
    
    EXPECT_TRUE(constWrapper.hasObject());
    EXPECT_EQ(constWrapper.propertyCount(), 2);
    EXPECT_EQ(constWrapper.data(0).toString(), "Const Test");
    EXPECT_EQ(constWrapper.data(1).toInt(), 172);
    EXPECT_EQ(constWrapper.dataType(0), DataInfo::String);
    EXPECT_EQ(constWrapper.dataType(1), DataInfo::Int);
    EXPECT_TRUE(constWrapper.variantObject().isValid());
}

TEST(GrpcObjectWrapperTests, NativeDataTest)
{
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    
    MasterObject obj;
    obj.set_name("Native");
    obj.set_height(168);
    obj.set_salary(55000.25);
    obj.set_married(true);
    
    wrapper.setObject(QVariant::fromValue(obj));
    
    // Test native data retrieval
    GrpcVariantGet nameData = wrapper.nativeData(0);
    EXPECT_EQ(std::get<std::reference_wrapper<const std::string>>(nameData).get(), "Native");
    
    GrpcVariantGet heightData = wrapper.nativeData(1);
    EXPECT_EQ(std::get<int32_t>(heightData), 168);
    
    GrpcVariantGet salaryData = wrapper.nativeData(2);
    EXPECT_DOUBLE_EQ(std::get<double>(salaryData), 55000.25);
    
    GrpcVariantGet marriedData = wrapper.nativeData(3);
    EXPECT_TRUE(std::get<bool>(marriedData));
}

TEST(GrpcObjectWrapperTests, AllPropertiesOfMasterObjectTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    GrpcObjectWrapper<MasterObject> wrapper;
    wrapper.addProperty("Uid", DataInfo::Int, &MasterObject::set_uid, &MasterObject::uid);
    wrapper.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    wrapper.addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
    wrapper.addProperty("Time", DataInfo::Int64, &MasterObject::set_time, &MasterObject::time);
    wrapper.addProperty("DateTime", DataInfo::DateTime, &MasterObject::set_date_time, &MasterObject::date_time);
    wrapper.addProperty("DateTimeNoSec", DataInfo::DateTimeNoSec, &MasterObject::set_date_time_no_sec, &MasterObject::date_time_no_sec);
    wrapper.addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
    wrapper.addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
    wrapper.addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
    wrapper.addProperty("Level", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);
    wrapper.addProperty("LevelName", DataInfo::String, &MasterObject::set_level_name, &MasterObject::level_name);
    wrapper.addProperty("MarriedName", DataInfo::String, &MasterObject::set_married_name, &MasterObject::married_name);
    wrapper.addProperty("Image", DataInfo::String, &MasterObject::set_image, &MasterObject::image);
    
    MasterObject obj;
    obj.set_uid(99);
    obj.set_name("CompleteTest");
    obj.set_date(current.toSecsSinceEpoch());
    obj.set_time(120000);
    obj.set_date_time(current.toSecsSinceEpoch());
    obj.set_date_time_no_sec(current.toSecsSinceEpoch());
    obj.set_height(178);
    obj.set_salary(88000.88);
    obj.set_married(true);
    obj.set_level(5);
    obj.set_level_name("Expert");
    obj.set_married_name("Married");
    obj.set_image("/images/test.jpg");
    
    wrapper.setObject(QVariant::fromValue(obj));
    
    EXPECT_EQ(wrapper.propertyCount(), 13);
    EXPECT_EQ(wrapper.data(0).toInt(), 99);
    EXPECT_EQ(wrapper.data(1).toString(), "CompleteTest");
    EXPECT_EQ(wrapper.data(2).toLongLong(), current.toSecsSinceEpoch());
    EXPECT_EQ(wrapper.data(3).toLongLong(), 120000);
    EXPECT_EQ(wrapper.data(6).toInt(), 178);
    EXPECT_DOUBLE_EQ(wrapper.data(7).toDouble(), 88000.88);
    EXPECT_TRUE(wrapper.data(8).toBool());
    EXPECT_EQ(wrapper.data(9).toInt(), 5);
    EXPECT_EQ(wrapper.data(10).toString(), "Expert");
    EXPECT_EQ(wrapper.data(11).toString(), "Married");
    EXPECT_EQ(wrapper.data(12).toString(), "/images/test.jpg");
}

TEST(GrpcObjectWrapperTests, SlaveObjectTest)
{
    GrpcObjectWrapper<SlaveObject> wrapper;
    wrapper.addProperty("Uid", DataInfo::Int, &SlaveObject::set_uid, &SlaveObject::uid);
    wrapper.addProperty("LinkUid", DataInfo::Int, &SlaveObject::set_link_uid, &SlaveObject::link_uid);
    wrapper.addProperty("Phone", DataInfo::String, &SlaveObject::set_phone, &SlaveObject::phone);
    
    SlaveObject obj(10, 100, "+1234567890");
    wrapper.setObject(QVariant::fromValue(obj));
    
    EXPECT_EQ(wrapper.propertyCount(), 3);
    EXPECT_EQ(wrapper.data(0).toInt(), 10);
    EXPECT_EQ(wrapper.data(1).toInt(), 100);
    EXPECT_EQ(wrapper.data(2).toString(), "+1234567890");
    
    // Update slave object data
    wrapper.setData(0, QVariant(20));
    wrapper.setData(1, QVariant(200));
    wrapper.setData(2, QVariant(QString("+9876543210")));
    
    EXPECT_EQ(wrapper.data(0).toInt(), 20);
    EXPECT_EQ(wrapper.data(1).toInt(), 200);
    EXPECT_EQ(wrapper.data(2).toString(), "+9876543210");
}

TEST(GrpcObjectWrapperTests, MoveConstructorTest)
{
    GrpcObjectWrapper<MasterObject> wrapper1;
    wrapper1.addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
    
    MasterObject obj;
    obj.set_name("MoveTest");
    wrapper1.setObject(QVariant::fromValue(obj));
    
    // Move constructor
    GrpcObjectWrapper<MasterObject> wrapper2(std::move(wrapper1));
    
    EXPECT_TRUE(wrapper2.hasObject());
    EXPECT_EQ(wrapper2.data(0).toString(), "MoveTest");
}
