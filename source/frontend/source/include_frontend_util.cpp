#include "include_frontend_util.h"

#include <QVariant>
#include <chrono>
#include <format>

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

QVariant to_qvariant_by_type(const GrpcVariantGet & varData, DataInfo::Type type) {
    const QVariant & qVariantData = to_qvariant_get(varData);
    std::string retData;
    switch(type) {
    case DataInfo::Double:
        // temporary
        return qVariantData;
    case DataInfo::DateTime:
    case DataInfo::DateTimeNoSec:
    case DataInfo::Date:
    case DataInfo::Time: {
        using namespace std::chrono;
        milliseconds ms_since_epoch{qVariantData.toLongLong()};
        sys_seconds tp_s = time_point_cast<seconds>(sys_time<milliseconds>{ms_since_epoch});
        std::chrono::zoned_time local{std::chrono::current_zone(), tp_s};
        std::locale loc(""); // system locale
        if(type == DataInfo::Date) {
            retData = std::format(loc, "{:L%x}", local);
        } else if(type == DataInfo::Time) {
            retData = std::format(loc, "{:L%X}", local);
        } else if(type == DataInfo::DateTime) {
            retData = std::format(loc, "{:L%c}", local);
        } else if(type == DataInfo::DateTimeNoSec) {
            retData = std::format(loc, "{:L%c}", local); // full locale date+time
            // Find last colon (start of seconds field in time portion)
            auto pos = retData.rfind(':');
            if (pos != std::string::npos && pos + 3 <= retData.size()) {
                retData.erase(pos, 3);  // erase ":ss"
            }
        }
        return QString::fromStdString(retData);
    }
    default:
        return qVariantData;
    }
}

}
