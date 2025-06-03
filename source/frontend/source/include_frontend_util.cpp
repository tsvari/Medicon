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
}
