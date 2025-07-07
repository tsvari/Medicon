#include "include_frontend_util.h"

#include <QVariant>

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
}
