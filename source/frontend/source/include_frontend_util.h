#pragma once

#include "include_util.h"

#include <QString>
#include <QItemSelection>
#include <QModelIndex>
#include <QVariant>

#ifdef Q_OS_MAC
//#define PROJECT_PATH "/Users/tsvari/tida-web/"
//#elif Q_OS_LINUX
//#define PROJECT_PATH "/home/vaky/tida-web/"
#endif

using GrpcVariantGet = std::variant<int32_t, std::reference_wrapper<const std::string>, int64_t, bool, double>;
namespace FrontConverter {

QString to_str(const std::string & source);
std::string to_str(const QString & source);
std::string to_str(const QVariant & source);

QVariant to_qvariant_get(GrpcVariantGet varData);
}






