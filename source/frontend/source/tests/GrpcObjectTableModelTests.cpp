#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GrpcObjectTableModel.h"
#include "TestSharedUtility.h"
#include "TypeToStringFormatter.h"
#include "GrpcDataContainer.hpp"

#include <QDateTime>


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

