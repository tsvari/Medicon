#include "gtest/gtest.h"

#include "include_frontend_util.h"

#include <QDateTime>
#include <QRegularExpression>

TEST(FrontConverterTests, ToQVariantByType_DateTime_DoesNotThrowAndNonEmpty)
{
    const qint64 ms = QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5), Qt::UTC).toMSecsSinceEpoch();

    EXPECT_NO_THROW({
        const QString text = FrontConverter::to_qvariant_by_type(QVariant::fromValue(ms), DataInfo::DateTime).toString();
        EXPECT_FALSE(text.isEmpty());
    });
}

TEST(FrontConverterTests, ToQVariantByType_Date_DoesNotThrowAndNonEmpty)
{
    const qint64 ms = QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5), Qt::UTC).toMSecsSinceEpoch();

    EXPECT_NO_THROW({
        const QString text = FrontConverter::to_qvariant_by_type(QVariant::fromValue(ms), DataInfo::Date).toString();
        EXPECT_FALSE(text.isEmpty());
    });
}

TEST(FrontConverterTests, ToQVariantByType_Time_DoesNotThrowAndNonEmpty)
{
    const qint64 ms = QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5), Qt::UTC).toMSecsSinceEpoch();

    EXPECT_NO_THROW({
        const QString text = FrontConverter::to_qvariant_by_type(QVariant::fromValue(ms), DataInfo::Time).toString();
        EXPECT_FALSE(text.isEmpty());
    });
}

TEST(FrontConverterTests, ToQVariantByType_DateTimeNoSec_DoesNotContainSecondsField)
{
    const qint64 ms = QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5), Qt::UTC).toMSecsSinceEpoch();

    EXPECT_NO_THROW({
        const QString text = FrontConverter::to_qvariant_by_type(QVariant::fromValue(ms), DataInfo::DateTimeNoSec).toString();
        EXPECT_FALSE(text.isEmpty());

        // Don't assert exact formatting (locale-dependent). Just ensure we don't have a "HH:MM:SS"-like component.
        const QRegularExpression secondsLike(R"(\b\d{1,2}[:\.]\d{2}[:\.]\d{2}\b)");
        EXPECT_FALSE(secondsLike.match(text).hasMatch());
    });
}
