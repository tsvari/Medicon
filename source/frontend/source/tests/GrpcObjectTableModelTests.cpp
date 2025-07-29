#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "GrpcObjectTableModel.h"
#include "TestSharedUtility.h"

#include <QDateTime>


using ::testing::ElementsAre;

namespace {
const int columnCount = 5;
}

TEST(GrpcObjectTableModelTests, GprcBasicTest)
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

    GrpcTestObjectTableModel model(std::move(objects));
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.columnCount(), columnCount);

    // First row
    auto actualRow1 = pullout<QString>(
        {model.index(0,0), model.index(0, model.columnCount() - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRow1, ElementsAre("Givi",
                                    std::to_string(current.toSecsSinceEpoch()),
                                    "168",
                                    "12.15",
                                    "false"));

    // Second row
    auto actualRow2 = pullout<QString>(
        {model.index(1,0), model.index(1, columnCount - 1)},
        Qt::DisplayRole
        );

    EXPECT_THAT(actualRow2, ElementsAre("Keto",
                                        std::to_string(current.toSecsSinceEpoch()),
                                        "164",
                                        "30.557",
                                        "true"));

    GprcTestDataObject obj3;
    obj3.set_name("Vakho");
    obj3.set_date(current.toSecsSinceEpoch());
    obj3.set_height(175);
    obj3.set_salary(135000.567);
    obj3.set_married(true);

    // Second Row again
    model.insertObject(1, QVariant::fromValue<GprcTestDataObject>(obj3));
    EXPECT_EQ(model.rowCount(), 3);
    EXPECT_EQ("Givi", model.index(0,0).data().toString());
    auto actualRowInserted = pullout<QString>(
        {model.index(1,0), model.index(1, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowInserted, ElementsAre("Vakho",
                                        std::to_string(current.toSecsSinceEpoch()),
                                        "175",
                                        "135000.567",
                                        "true"));

    GprcTestDataObject obj4;
    obj4.set_name("Elene");
    obj4.set_date(current.toSecsSinceEpoch());
    obj4.set_height(155);
    obj4.set_salary(567);
    obj4.set_married(false);

    // Forth row
    model.addNewObject(QVariant::fromValue<GprcTestDataObject>(obj4));
    EXPECT_EQ(model.rowCount(), 4);

    auto actualRowAdded = pullout<QString>(
        {model.index(3,0), model.index(3, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowAdded, ElementsAre("Elene",
                                               std::to_string(current.toSecsSinceEpoch()),
                                               "155",
                                               "567",
                                               "false"));
    // Check names only first column
    auto actualNamesFirstColumn = pullout<QString>(
        {model.index(0,0), model.index(3, 0)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                            "Vakho",
                                            "Keto",
                                            "Elene"));
    // Update object test
    GprcTestDataObject newObj;
    newObj.set_name("Teona");
    newObj.set_date(current.toSecsSinceEpoch());
    newObj.set_height(166);
    newObj.set_salary(5.123);
    newObj.set_married(true);

    model.updateObject(3, QVariant::fromValue<GprcTestDataObject>(newObj));
    auto actualRowUpdated = pullout<QString>(
        {model.index(3,0), model.index(3, columnCount - 1)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualRowUpdated, ElementsAre("Teona",
                                            std::to_string(current.toSecsSinceEpoch()),
                                            "166",
                                            "5.123",
                                            "true"));

    // Be sure it's update not insert
    EXPECT_EQ(model.rowCount(), 4);
    actualNamesFirstColumn = pullout<QString>(
        {model.index(0,0), model.index(3, 0)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho",
                                                    "Keto",
                                                    "Teona"));

    // Header tests
    auto actualHeadersHorizontal = pulloutHeader<QString>(&model, {0, 1, 2, 3, 4}, Qt::Horizontal, Qt::DisplayRole);
    EXPECT_THAT(actualHeadersHorizontal, ElementsAre(
                                        "Name",
                                        "Date",
                                        "Height",
                                        "Salary",
                                        "Married"));

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
        {model.index(0,0), model.index(2, 0)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho",
                                                    "Teona"));

    // Remove last object / Teona
    model.deleteObject(2);
    EXPECT_EQ(model.rowCount(), 2);
    actualNamesFirstColumn = pullout<QString>(
        {model.index(0,0), model.index(1, 0)},
        Qt::DisplayRole
        );
    EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
                                                    "Vakho"));

    // Remove First object / Givi
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_THAT(model.index(0, 0).data().toString(), "Vakho");

    // Remove last one
    model.deleteObject(0);
    EXPECT_EQ(model.rowCount(), 0);
}
