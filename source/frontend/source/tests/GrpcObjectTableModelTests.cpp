#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GrpcObjectTableModel.h"
#include "TestSharedUtility.h"
#include "TypeToStringFormatter.h"
#include "GrpcDataContainer.hpp"

#include <QDateTime>
#include <QSignalSpy>


using ::testing::ElementsAre;


class GrpcTestObjectTableModel : public GrpcObjectTableModel
{

public:
    explicit GrpcTestObjectTableModel(std::vector<MasterObject> && data, QObject *parent = nullptr) :
        GrpcObjectTableModel(new GrpcDataContainer<MasterObject>(std::move(data)), parent)
    {
        initializeModel();
        initializeContainer();
    }

    explicit GrpcTestObjectTableModel(QObject *parent = nullptr) :
        GrpcObjectTableModel( new GrpcDataContainer<MasterObject>(), parent)
    {
        initializeModel();
        initializeContainer();
    }

    void initializeModel() override {
        GrpcDataContainer<MasterObject> * container = dynamic_cast<GrpcDataContainer<MasterObject>*>(objectContainer());

        container->addProperty("Uid", DataInfo::String, &MasterObject::set_uid, &MasterObject::uid);
        container->addProperty("Name", DataInfo::String, &MasterObject::set_name, &MasterObject::name);
        container->addProperty("Date", DataInfo::Date, &MasterObject::set_date, &MasterObject::date);
        container->addProperty("Height", DataInfo::Int, &MasterObject::set_height, &MasterObject::height);
        container->addProperty("Salary", DataInfo::Double, &MasterObject::set_salary, &MasterObject::salary);
        container->addProperty("Married", DataInfo::Bool, &MasterObject::set_married, &MasterObject::married);
        container->addProperty("Married Name", DataInfo::String, &MasterObject::set_married_name, &MasterObject::married_name);
        container->addProperty("Level", DataInfo::Int, &MasterObject::set_level, &MasterObject::level);
        container->addProperty("Level Name", DataInfo::String, &MasterObject::set_level_name, &MasterObject::level_name);
    }
};

namespace {
void compareObjects (const MasterObject & left, const MasterObject & right) {
    EXPECT_EQ(left.uid(), right.uid());
    EXPECT_EQ(left.name(), right.name());
    EXPECT_EQ(TimeFormatHelper::chronoSysSecToString(left.date(), DataInfo::Date), TimeFormatHelper::chronoSysSecToString(right.date(), DataInfo::Date));
    EXPECT_EQ(left.height(), right.height());
    EXPECT_EQ(left.salary(), right.salary());
    EXPECT_EQ(left.married(), right.married());
}
const int columnCount = 9;
}

// Need to test setData for GrpcObjectTableModel
// Use the same pattern as in GrpcDataContainerTests

TEST(GrpcObjectTableModelTests, GprcBasicTest)
{
    QDateTime current = QDateTime::currentDateTime();

    MasterObject obj1;

    obj1.set_uid(1);
    obj1.set_name("Givi");
    obj1.set_date(current.toSecsSinceEpoch());
    obj1.set_height(168);
    obj1.set_salary(12.15);
    obj1.set_married(false);
    obj1.set_married_name("No");
    obj1.set_level(2);
    obj1.set_level_name("Level2");

    MasterObject obj2;

    obj2.set_uid(2);
    obj2.set_name("Keto");
    obj2.set_date(current.toSecsSinceEpoch());
    obj2.set_height(164);
    obj2.set_salary(30.557);
    obj2.set_married(true);
    obj2.set_married_name("Yes");
    obj2.set_level(1);
    obj2.set_level_name("Level1");

    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);

    GrpcTestObjectTableModel model(std::move(objects));
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.columnCount(), columnCount);

    QVariant variantObject1 = model.variantObject(0);
    EXPECT_TRUE(variantObject1.isValid());
    MasterObject expectedRealObject1 = variantObject1.value<MasterObject>();
    compareObjects(obj1, expectedRealObject1);

    QVariant variantObject2 = model.variantObject(1);
    EXPECT_TRUE(variantObject2.isValid());
    MasterObject expectedRealObject2 = variantObject2.value<MasterObject>();
    compareObjects(obj2, expectedRealObject2);

    // First row
    auto actualRow1 = pullout<QString>(
        {model.index(0,0), model.index(0, model.columnCount() - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRow1, ElementsAre("1",
                                    "Givi",
                                    FrontConverter::to_qvariant_get_by_type(current.toSecsSinceEpoch(), DataInfo::Date),
                                    "168",
                                    FrontConverter::to_qvariant_get_by_type(12.15, DataInfo::Double),
                                    "false", "No",
                                    "2", "Level2"));

    // Second row
    auto actualRow2 = pullout<QString>(
        {model.index(1,0), model.index(1, columnCount - 1)},
        Qt::DisplayRole
        );

    EXPECT_THAT(actualRow2, ElementsAre("2",
                                        "Keto",
                                        FrontConverter::to_qvariant_get_by_type(current.toSecsSinceEpoch(), DataInfo::Date),
                                        "164",
                                        FrontConverter::to_qvariant_get_by_type(30.557, DataInfo::Double),
                                        "true", "Yes",
                                        "1", "Level1"));

    MasterObject obj3;
    obj3.set_uid(3);
    obj3.set_name("Vakho");
    obj3.set_date(current.toSecsSinceEpoch());
    obj3.set_height(175);
    obj3.set_salary(135000.567);
    obj3.set_married(true);
    obj3.set_married_name("Yes");
    obj3.set_level(3);
    obj3.set_level_name("Level3");

    // Second Row again
    model.insertObject(1, QVariant::fromValue<MasterObject>(obj3));
    EXPECT_EQ(model.rowCount(), 3);
    EXPECT_EQ("Givi", model.index(0,1).data().toString());
    auto actualRowInserted = pullout<QString>(
        {model.index(1,0), model.index(1, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowInserted, ElementsAre("3",
                                       "Vakho",
                                        FrontConverter::to_qvariant_get_by_type(current.toSecsSinceEpoch(), DataInfo::Date),
                                        "175",
                                        FrontConverter::to_qvariant_get_by_type(135000.567, DataInfo::Double),
                                        "true", "Yes", "3", "Level3"));

    MasterObject obj4;
    obj4.set_uid(4);
    obj4.set_name("Elene");
    obj4.set_date(current.toSecsSinceEpoch());
    obj4.set_height(155);
    obj4.set_salary(567);
    obj4.set_married(false);
    obj4.set_married_name("No");
    obj4.set_level(5);
    obj4.set_level_name("Level5");

    // Forth row
    model.addNewObject(QVariant::fromValue<MasterObject>(obj4));
    EXPECT_EQ(model.rowCount(), 4);

    auto actualRowAdded = pullout<QString>(
        {model.index(3,0), model.index(3, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowAdded, ElementsAre("4",
                                            "Elene",
                                            FrontConverter::to_qvariant_get_by_type(current.toSecsSinceEpoch(), DataInfo::Date),
                                            "155",
                                            FrontConverter::to_qvariant_get_by_type(567, DataInfo::Double),
                                            "false", "No", "5", "Level5"));

    // Check names only first column
    auto actualNamesFirstColumn = pullout<QString>(
        {model.index(0,1), model.index(3, 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                            "Vakho",
                                            "Keto",
                                            "Elene"));

    // Update object test
    MasterObject newObj;
    newObj.set_uid(5);
    newObj.set_name("Teona");
    newObj.set_date(current.toSecsSinceEpoch());
    newObj.set_height(166);
    newObj.set_salary(5.123);
    newObj.set_married(true);
    newObj.set_married_name("Yes");
    newObj.set_level(4);
    newObj.set_level_name("Level4");

    model.updateObject(3, QVariant::fromValue<MasterObject>(newObj));
    auto actualRowUpdated = pullout<QString>(
        {model.index(3,0), model.index(3, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowUpdated, ElementsAre("5",
                                            "Teona",
                                            FrontConverter::to_qvariant_get_by_type(current.toSecsSinceEpoch(), DataInfo::Date),
                                            "166",
                                            FrontConverter::to_qvariant_get_by_type(5.123, DataInfo::Double),
                                            "true", "Yes", "4", "Level4"));


    // Be sure it's update not insert
    EXPECT_EQ(model.rowCount(), 4);
    actualNamesFirstColumn = pullout<QString>(
        {model.index(0,1), model.index(3, 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho",
                                                    "Keto",
                                                    "Teona"));

    // Header tests
    auto actualHeadersHorizontal = pulloutHeader<QString>(&model, {0, 1, 2, 3, 4, 5, 6, 7, 8}, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_THAT(actualHeadersHorizontal, ElementsAre(
                                        "Uid",
                                        "Name",
                                        "Date",
                                        "Height",
                                        "Salary",
                                        "Married", "Married Name", "Level", "Level Name"));

    auto actualHeadersVertical = pulloutHeader<QString>(&model, {0, 1, 2, 3}, Qt::Vertical, Qt::DisplayRole);
    EXPECT_THAT(actualHeadersVertical, ElementsAre(
                                             "1",
                                             "2",
                                             "3",
                                             "4"));

    // Test Remove
    // Remove third object / Keto
    model.deleteObject(2);
    EXPECT_EQ(model.rowCount(), 3);
    actualNamesFirstColumn = pullout<QString>(
        {model.index(0,1), model.index(2, 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho",
                                                    "Teona"));

    // Remove last object / Teona
    model.deleteObject(2);
    EXPECT_EQ(model.rowCount(), 2);
    actualNamesFirstColumn = pullout<QString>(
        {model.index(0,1), model.index(1, 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho"));

    // Remove First object / Givi
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_THAT(model.index(0, 1).data().toString(), "Vakho");

    // Remove last one
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 0);

}

// ============================================================================
// Additional Tests for Improved GrpcObjectTableModel
// ============================================================================

TEST(GrpcObjectTableModelTests, EmptyModelTest)
{
    GrpcTestObjectTableModel model;
    
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_EQ(model.columnCount(), columnCount);
}

TEST(GrpcObjectTableModelTests, InvalidIndexTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    MasterObject obj;
    obj.set_name("Test");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Invalid index
    QModelIndex invalidIndex;
    EXPECT_FALSE(invalidIndex.isValid());
    EXPECT_FALSE(model.data(invalidIndex).isValid());
    
    // Out of bounds
    QModelIndex outOfBounds = model.index(10, 0);
    EXPECT_FALSE(outOfBounds.isValid());
}

TEST(GrpcObjectTableModelTests, SetDataTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    MasterObject obj;
    obj.set_name("Original");
    obj.set_height(170);
    obj.set_salary(50000.0);
    obj.set_married(false);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Set name
    QModelIndex nameIndex = model.index(0, 1);
    EXPECT_TRUE(model.setData(nameIndex, "Updated", Qt::EditRole));
    EXPECT_EQ(model.data(nameIndex).toString(), "Updated");
    
    // Set height
    QModelIndex heightIndex = model.index(0, 3);
    EXPECT_TRUE(model.setData(heightIndex, 180, Qt::EditRole));
    EXPECT_EQ(model.data(heightIndex).toString(), "180");
    
    // Set salary
    QModelIndex salaryIndex = model.index(0, 4);
    EXPECT_TRUE(model.setData(salaryIndex, 60000.50, Qt::EditRole));
    EXPECT_EQ(model.data(salaryIndex).toString(), FrontConverter::to_qvariant_get_by_type(60000.50, DataInfo::Double));
    
    // Set married
    QModelIndex marriedIndex = model.index(0, 5);
    EXPECT_TRUE(model.setData(marriedIndex, true, Qt::EditRole));
    EXPECT_EQ(model.data(marriedIndex).toString(), "true");
}

TEST(GrpcObjectTableModelTests, SetDataNoChangeTest)
{
    MasterObject obj;
    obj.set_name("Test");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QModelIndex nameIndex = model.index(0, 1);
    
    // Setting same value should return false
    EXPECT_FALSE(model.setData(nameIndex, "Test", Qt::EditRole));
}

TEST(GrpcObjectTableModelTests, FlagsTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QModelIndex validIndex = model.index(0, 0);
    Qt::ItemFlags flags = model.flags(validIndex);
    
    EXPECT_TRUE(flags & Qt::ItemIsEnabled);
    EXPECT_TRUE(flags & Qt::ItemIsSelectable);
    EXPECT_TRUE(flags & Qt::ItemIsEditable);
    
    QModelIndex invalidIndex;
    EXPECT_EQ(model.flags(invalidIndex), Qt::NoItemFlags);
}

TEST(GrpcObjectTableModelTests, HeaderDataHorizontalTest)
{
    GrpcTestObjectTableModel model;
    
    EXPECT_EQ(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Uid");
    EXPECT_EQ(model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "Name");
    EXPECT_EQ(model.headerData(2, Qt::Horizontal, Qt::DisplayRole).toString(), "Date");
    EXPECT_EQ(model.headerData(3, Qt::Horizontal, Qt::DisplayRole).toString(), "Height");
    EXPECT_EQ(model.headerData(4, Qt::Horizontal, Qt::DisplayRole).toString(), "Salary");
    EXPECT_EQ(model.headerData(5, Qt::Horizontal, Qt::DisplayRole).toString(), "Married");
}

TEST(GrpcObjectTableModelTests, HeaderDataVerticalTest)
{
    MasterObject obj1, obj2, obj3;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    objects.push_back(obj2);
    objects.push_back(obj3);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    EXPECT_EQ(model.headerData(0, Qt::Vertical, Qt::DisplayRole).toInt(), 1);
    EXPECT_EQ(model.headerData(1, Qt::Vertical, Qt::DisplayRole).toInt(), 2);
    EXPECT_EQ(model.headerData(2, Qt::Vertical, Qt::DisplayRole).toInt(), 3);
}

TEST(GrpcObjectTableModelTests, HeaderDataAlignmentTest)
{
    GrpcTestObjectTableModel model;
    
    // String (Name) - Left aligned
    QVariant nameAlign = model.headerData(1, Qt::Horizontal, Qt::TextAlignmentRole);
    EXPECT_EQ(nameAlign.value<Qt::Alignment>(), Qt::AlignLeft | Qt::AlignVCenter);
    
    // Date - Center aligned
    QVariant dateAlign = model.headerData(2, Qt::Horizontal, Qt::TextAlignmentRole);
    EXPECT_EQ(dateAlign.value<Qt::Alignment>(), Qt::AlignCenter);
    
    // Int (Height) - Right aligned
    QVariant heightAlign = model.headerData(3, Qt::Horizontal, Qt::TextAlignmentRole);
    EXPECT_EQ(heightAlign.value<Qt::Alignment>(), Qt::AlignRight | Qt::AlignVCenter);
    
    // Double (Salary) - Right aligned
    QVariant salaryAlign = model.headerData(4, Qt::Horizontal, Qt::TextAlignmentRole);
    EXPECT_EQ(salaryAlign.value<Qt::Alignment>(), Qt::AlignRight | Qt::AlignVCenter);
    
    // Bool (Married) - Center aligned
    QVariant marriedAlign = model.headerData(5, Qt::Horizontal, Qt::TextAlignmentRole);
    EXPECT_EQ(marriedAlign.value<Qt::Alignment>(), Qt::AlignCenter);
}

TEST(GrpcObjectTableModelTests, DataAlignmentRoleTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // String - Left aligned
    QVariant nameAlign = model.data(model.index(0, 1), Qt::TextAlignmentRole);
    EXPECT_EQ(nameAlign.value<Qt::Alignment>(), Qt::AlignLeft | Qt::AlignVCenter);
    
    // Int - Right aligned
    QVariant heightAlign = model.data(model.index(0, 3), Qt::TextAlignmentRole);
    EXPECT_EQ(heightAlign.value<Qt::Alignment>(), Qt::AlignRight | Qt::AlignVCenter);
    
    // Bool - Center aligned
    QVariant marriedAlign = model.data(model.index(0, 5), Qt::TextAlignmentRole);
    EXPECT_EQ(marriedAlign.value<Qt::Alignment>(), Qt::AlignCenter);
}

TEST(GrpcObjectTableModelTests, VariantObjectRoleTest)
{
    MasterObject obj;
    obj.set_uid(42);
    obj.set_name("TestObject");
    obj.set_height(175);
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QVariant variantData = model.data(model.index(0, 0), GlobalRoles::VariantObjectRole);
    EXPECT_TRUE(variantData.isValid());
    
    MasterObject retrieved = variantData.value<MasterObject>();
    EXPECT_EQ(retrieved.uid(), 42);
    EXPECT_EQ(retrieved.name(), "TestObject");
    EXPECT_EQ(retrieved.height(), 175);
}

TEST(GrpcObjectTableModelTests, VariantObjectMethodTest)
{
    MasterObject obj;
    obj.set_uid(99);
    obj.set_name("DirectMethod");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QVariant variantData = model.variantObject(0);
    EXPECT_TRUE(variantData.isValid());
    
    MasterObject retrieved = variantData.value<MasterObject>();
    EXPECT_EQ(retrieved.uid(), 99);
    EXPECT_EQ(retrieved.name(), "DirectMethod");
}



TEST(GrpcObjectTableModelTests, InsertAtBeginningTest)
{
    MasterObject obj1, obj2;
    obj1.set_name("First");
    obj2.set_name("Second");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj1);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    model.insertObject(0, QVariant::fromValue(obj2));
    
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.index(0, 1).data().toString(), "Second");
    EXPECT_EQ(model.index(1, 1).data().toString(), "First");
}







TEST(GrpcObjectTableModelTests, MultipleInsertsTest)
{
    GrpcTestObjectTableModel model;
    
    EXPECT_EQ(model.rowCount(), 0);
    
    for (int i = 0; i < 5; ++i) {
        MasterObject obj;
        obj.set_name("Object" + std::to_string(i));
        model.addNewObject(QVariant::fromValue(obj));
    }
    
    EXPECT_EQ(model.rowCount(), 5);
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(model.index(i, 1).data().toString(), QString::fromStdString("Object" + std::to_string(i)));
    }
}

TEST(GrpcObjectTableModelTests, SignalInsertedTest)
{
    GrpcTestObjectTableModel model;
    
    QSignalSpy spy(&model, &GrpcObjectTableModel::inserted);
    
    MasterObject obj;
    model.addNewObject(QVariant::fromValue(obj));
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), 0);
}

TEST(GrpcObjectTableModelTests, SignalUpdatedTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QSignalSpy spy(&model, &GrpcObjectTableModel::updated);
    
    MasterObject newObj;
    model.updateObject(0, QVariant::fromValue(newObj));
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), 0);
}

TEST(GrpcObjectTableModelTests, SignalDeletedTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QSignalSpy spy(&model, &GrpcObjectTableModel::deleted);
    
    model.deleteObject(0);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toInt(), 0);
}

TEST(GrpcObjectTableModelTests, SignalZeroCountTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QSignalSpy spy(&model, &GrpcObjectTableModel::zeroCount);
    
    model.deleteObject(0);
    
    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_TRUE(arguments.at(0).toBool());
}

// Exception Tests

TEST(GrpcObjectTableModelTests, InsertObjectThrowsOnInvalidRow)
{
    GrpcTestObjectTableModel model;
    
    MasterObject obj;
    
    // Insert at negative row
    EXPECT_THROW({
        model.insertObject(-1, QVariant::fromValue(obj));
    }, std::out_of_range);
    
    // Insert beyond count
    EXPECT_THROW({
        model.insertObject(2, QVariant::fromValue(obj));
    }, std::out_of_range);
}

TEST(GrpcObjectTableModelTests, InsertObjectThrowsOnInvalidData)
{
    GrpcTestObjectTableModel model;
    
    // Insert invalid QVariant
    EXPECT_THROW({
        model.insertObject(0, QVariant());
    }, std::invalid_argument);
}

TEST(GrpcObjectTableModelTests, UpdateObjectThrowsOnInvalidRow)
{
    MasterObject obj;
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    MasterObject newObj;
    newObj.set_name("Updated");
    
    // Update at negative row
    EXPECT_THROW({
        model.updateObject(-1, QVariant::fromValue(newObj));
    }, std::out_of_range);
    
    // Update beyond count
    EXPECT_THROW({
        model.updateObject(1, QVariant::fromValue(newObj));
    }, std::out_of_range);
}

TEST(GrpcObjectTableModelTests, UpdateObjectThrowsOnInvalidData)
{
    MasterObject obj;
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Update with invalid QVariant
    EXPECT_THROW({
        model.updateObject(0, QVariant());
    }, std::invalid_argument);
}

TEST(GrpcObjectTableModelTests, DeleteObjectThrowsOnInvalidRow)
{
    MasterObject obj;
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Delete at negative row
    EXPECT_THROW({
        model.deleteObject(-1);
    }, std::out_of_range);
    
    // Delete beyond count
    EXPECT_THROW({
        model.deleteObject(1);
    }, std::out_of_range);
}

TEST(GrpcObjectTableModelTests, VariantObjectThrowsOnInvalidRow)
{
    MasterObject obj;
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Access at negative row
    EXPECT_THROW({
        model.variantObject(-1);
    }, std::out_of_range);
    
    // Access beyond count
    EXPECT_THROW({
        model.variantObject(1);
    }, std::out_of_range);
}

TEST(GrpcObjectTableModelTests, ExceptionMessagesAreDescriptive)
{
    GrpcTestObjectTableModel model;
    
    try {
        model.insertObject(-1, QVariant::fromValue(MasterObject()));
        FAIL() << "Expected std::out_of_range exception";
    } catch (const std::out_of_range& e) {
        std::string message = e.what();
        EXPECT_TRUE(message.find("Row index out of range") != std::string::npos);
        EXPECT_TRUE(message.find("-1") != std::string::npos);
    }
    
    try {
        model.insertObject(0, QVariant());
        FAIL() << "Expected std::invalid_argument exception";
    } catch (const std::invalid_argument& e) {
        std::string message = e.what();
        EXPECT_TRUE(message.find("invalid") != std::string::npos);
    }
}

TEST(GrpcObjectTableModelTests, DataChangedSignalTest)
{
    MasterObject obj;
    obj.set_name("Original");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    QSignalSpy spy(&model, &QAbstractTableModel::dataChanged);
    
    model.setData(model.index(0, 1), "Updated", Qt::EditRole);
    
    EXPECT_GE(spy.count(), 1);
}

TEST(GrpcObjectTableModelTests, ParentIndexTest)
{
    MasterObject obj;
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Table models return 0 for valid parent
    QModelIndex validParent = model.index(0, 0);
    EXPECT_EQ(model.rowCount(validParent), 0);
    EXPECT_EQ(model.columnCount(validParent), 0);
}

TEST(GrpcObjectTableModelTests, RowCountConsistencyTest)
{
    GrpcTestObjectTableModel model;
    
    EXPECT_EQ(model.rowCount(), 0);
    
    MasterObject obj1;
    model.addNewObject(QVariant::fromValue(obj1));
    EXPECT_EQ(model.rowCount(), 1);
    
    MasterObject obj2;
    model.addNewObject(QVariant::fromValue(obj2));
    EXPECT_EQ(model.rowCount(), 2);
    
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 1);
    
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 0);
}

TEST(GrpcObjectTableModelTests, AllDataTypesDisplayTest)
{
    QDateTime current = QDateTime::currentDateTime();
    
    MasterObject obj;
    obj.set_uid(123);
    obj.set_name("AllTypes");
    obj.set_date(current.toSecsSinceEpoch());
    obj.set_height(180);
    obj.set_salary(75000.99);
    obj.set_married(true);
    obj.set_married_name("Yes");
    obj.set_level(3);
    obj.set_level_name("Senior");
    
    std::vector<MasterObject> objects;
    objects.push_back(obj);
    
    GrpcTestObjectTableModel model(std::move(objects));
    
    // Verify all data is displayed correctly
    EXPECT_EQ(model.index(0, 0).data().toString(), "123");  // Uid
    EXPECT_EQ(model.index(0, 1).data().toString(), "AllTypes");  // Name
    EXPECT_EQ(model.index(0, 3).data().toString(), "180");  // Height
    EXPECT_EQ(model.index(0, 5).data().toString(), "true");  // Married
    EXPECT_EQ(model.index(0, 6).data().toString(), "Yes");  // Married Name
    EXPECT_EQ(model.index(0, 7).data().toString(), "3");  // Level
    EXPECT_EQ(model.index(0, 8).data().toString(), "Senior");  // Level Name
}

