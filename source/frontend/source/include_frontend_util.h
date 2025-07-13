#pragma once

#include "include_util.h"

#include <QString>
#include <QItemSelection>
#include <QModelIndex>
#include <QVariant>

#ifdef Q_OS_MAC
//
//
#endif

namespace {
using GrpcVariantGet = std::variant<int32_t, std::reference_wrapper<const std::string>, int64_t, bool, double>;
using GrpcVariantSet = std::variant<int32_t, std::string, int64_t, bool, double>;

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;
}

namespace FrontConverter {

QString to_str(const std::string & source);
std::string to_str(const QString & source);
std::string to_str(const QVariant & source);

QVariant to_qvariant_get(GrpcVariantGet varData);
}






