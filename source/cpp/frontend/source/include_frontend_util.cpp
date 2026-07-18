#include "include_frontend_util.h"

#include <QVariant>
#include <QDateTime>
#include <QLocale>
#include <iostream>

namespace {

QString stripSecondsFromQtTimeFormat(QString format)
{
    for (;;) {
        int pos = format.indexOf("ss");
        int len = 2;
        if (pos < 0) {
            pos = format.indexOf('s');
            len = 1;
        }
        if (pos < 0) {
            break;
        }

        if (pos > 0) {
            const QChar prev = format.at(pos - 1);
            if (prev == ':' || prev == '.' || prev == ' ') {
                format.remove(pos - 1, len + 1);
                continue;
            }
        }
        format.remove(pos, len);
    }

    while (format.contains("::")) {
        format.replace("::", ":");
    }
    while (format.contains("..")) {
        format.replace("..", ".");
    }

    return format.trimmed();
}

}

namespace FrontConverter {
QString to_str(const std::string & source) {
    return QString::fromStdString(source);
}
std::string to_str(const QString & source) {
    return source.toStdString();
}
std::string to_str(const QVariant & source) {
    return source.toString().toStdString();
}

QVariant to_qvariant_get(GrpcVariantGet varData) {
    if (int32_t * ptr = std::get_if<int32_t>(&varData)) {
        return QVariant::fromValue(*ptr);
    } else if (int64_t * ptr = std::get_if<int64_t>(&varData)) {
        return QVariant::fromValue(*ptr);
    } else if (double * ptr = std::get_if<double>(&varData)) {
        return QVariant::fromValue(*ptr);
    } else if (auto ptr = std::get_if<std::reference_wrapper<const std::string>>(&varData)) {
        return QString::fromStdString(ptr->get());
    } else if (bool * ptr = std::get_if<bool>(&varData)) {
        return QVariant::fromValue(*ptr);
    } else {
        return QVariant();
    }
}

std::string to_locale_string(
    double value,
    int decimals = -1,                          // -1 means "full precision"
    const std::locale& loc = std::locale("")    // system/user locale
    ) {
    std::ostringstream oss;
    oss.imbue(loc);

    if (decimals >= 0) {
        // Fixed number of decimal places
        oss << std::fixed << std::setprecision(decimals);
    } else {
        // Enough precision for exact round-trip
        oss << std::setprecision(std::numeric_limits<double>::max_digits10);
    }

    oss << value;
    return oss.str();
}

QVariant to_qvariant_get_by_type(const GrpcVariantGet & varData, DataInfo::Type type) {
    const QVariant & qVariantData = to_qvariant_get(varData);
    return to_qvariant_by_type(qVariantData, type);
}

QVariant to_qvariant_by_type(const QVariant & qVariantData, DataInfo::Type type)
{
    std::string retData;
    switch(type) {
    case DataInfo::Double:
        return QString::fromStdString(to_locale_string(qVariantData.toDouble(), 3));
    case DataInfo::DateTime:
    case DataInfo::DateTimeNoSec:
    case DataInfo::Date:
    case DataInfo::Time: {
        const QLocale locale = QLocale::system();
        const qint64 msSinceEpoch = qVariantData.toLongLong();

        const QDateTime utc = QDateTime::fromMSecsSinceEpoch(msSinceEpoch, Qt::UTC);
        const QDateTime local = utc.toLocalTime();

        if (type == DataInfo::Date) {
            return locale.toString(local.date(), QLocale::ShortFormat);
        }
        if (type == DataInfo::Time) {
            return locale.toString(local.time(), QLocale::ShortFormat);
        }
        if (type == DataInfo::DateTime) {
            return locale.toString(local, QLocale::ShortFormat);
        }

        QString fmt = locale.dateTimeFormat(QLocale::ShortFormat);
        fmt = stripSecondsFromQtTimeFormat(std::move(fmt));
        return locale.toString(local, fmt);
    }
    default:
        return qVariantData;
    }
}

double to_locale_double(const QString & strValue)
{
    return QLocale::system().toDouble(strValue);
}

}
