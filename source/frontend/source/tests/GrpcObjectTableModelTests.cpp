#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TypeToStringFormatter.h"
#include "JsonParameterFormatter.h"
#include "include_frontend_util.h"
#include "TestSharedUtility.h"

#include "front_common.h"
#include <functional>

#include <QDateTime>
#include "GrpcObjectTableModel.h"

TEST(GrpcObjectTableModelTests, GprcBasicTest)
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

    GrpcObjectTableModel model(&container);
}
