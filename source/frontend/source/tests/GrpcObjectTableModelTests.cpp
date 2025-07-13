#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TypeToStringFormatter.h"
#include "JsonParameterFormatter.h"
#include "include_frontend_util.h"
#include "TestSharedUtility.h"

#include "front_common.h"
#include <functional>

#include <QDateTime>
#include <QString>
#include "GrpcObjectTableModel.h"

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

    //QItemSelection ss({model.index(0,0), model.index(3, 4)});
    //QModelIndexList ssss = ss.indexes();
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
    //QItemSelection ll({model.index(0,0), model.index(3, 0)});
    //QModelIndexList llll = ll.indexes();
    //auto actualNamesFirstColumn = pullout<QString>(
    //    {model.index(0,0), model.index(3, 0)},
    //    Qt::DisplayRole
    //    );
    //EXPECT_THAT(actualNamesFirstColumn, ElementsAre("Givi",
    //                                        "Vakho",
    //                                        "Keto",
    //                                        "Elene"));

    //EXPECT_EQ("Givi", model.index(0,0).data().toString());
    //EXPECT_EQ("Vakho", model.index(1,0).data().toString());
    //EXPECT_EQ("Keto", model.index(2,0).data().toString());
    //EXPECT_EQ("Elene", model.index(3,0).data().toString());
}
