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
