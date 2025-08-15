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

template<typename GrpcObject>
class Property
{
public:
    using Grpc32Set = void (GrpcObject::*)(int32_t);
    using Grpc32Get = int32_t (GrpcObject::*)()const;

    using Grpc64Set = void (GrpcObject::*)(int64_t);
    using Grpc64Get = int64_t (GrpcObject::*)()const;

    using GrpcStrSet = void (GrpcObject::*)(const string &);
    using GrpcStrGet = const std::string&(GrpcObject::*)()const;

    using GrpcBoolSet = void (GrpcObject::*)(bool);
    using GrpcBoolGet = bool (GrpcObject::*)()const;

    using GrpcDoubleSet = void (GrpcObject::*)(double);
    using GrpcDoubleGet = double (GrpcObject::*)()const;
    string name;  // Header data
    DataInfo::Type type;
    std::variant<
        GrpcStrSet,
        Grpc64Set,
        Grpc32Set,
        GrpcBoolSet,
        GrpcDoubleSet
        > setter;
    std::variant<
        GrpcStrGet,
        Grpc64Get,
        Grpc32Get,
        GrpcBoolGet,
        GrpcDoubleGet
        > getter;
};

struct PropertyHolder
{
    std::variant<
        std::function<void (const string &)>,
        std::function<void (int64_t)>,
        std::function<void (int32_t)>,
        std::function<void (bool)>,
        std::function<void (double)>
        > setter;
    std::variant<
        std::function<const std::string&(void)>,
        std::function<int64_t(void)>,
        std::function<int32_t(void)>,
        std::function<bool(void)>,
        std::function<double(void)>
        > getter;
};

template<typename GrpcObject>
static std::vector<PropertyHolder> bindSettersGetters2(GrpcObject * object, const std::vector<Property<GrpcObject>> & properties)
{
    using Grpc32Set = void (GrpcObject::*)(int32_t);
    using Grpc32Get = int32_t (GrpcObject::*)()const;

    using Grpc64Set = void (GrpcObject::*)(int64_t);
    using Grpc64Get = int64_t (GrpcObject::*)()const;

    using GrpcStrSet = void (GrpcObject::*)(const string &);
    using GrpcStrGet = const std::string&(GrpcObject::*)()const;

    using GrpcBoolSet = void (GrpcObject::*)(bool);
    using GrpcBoolGet = bool (GrpcObject::*)()const;

    using GrpcDoubleSet = void (GrpcObject::*)(double);
    using GrpcDoubleGet = double (GrpcObject::*)()const;

    std::vector<PropertyHolder> propertiesList;
    for(auto & property: properties) {
        PropertyHolder propertyObject{};
        // bind setters
        if (GrpcStrSet * ptr = std::get_if<GrpcStrSet>(&property.setter)) {
            propertyObject.setter = std::function<void (const string &)>(
                std::bind(*ptr,  object, std::placeholders::_1)
                );
        } else if (Grpc64Set * ptr = std::get_if<Grpc64Set>(&property.setter)) {
            propertyObject.setter = std::function<void (int64_t)>(
                std::bind(*ptr,  object, std::placeholders::_1)
                );
        } else if (Grpc32Set * ptr = std::get_if<Grpc32Set>(&property.setter)) {
            propertyObject.setter = std::function<void (int32_t)>(
                std::bind(*ptr, object, std::placeholders::_1)
                );
        } else if (GrpcBoolSet * ptr = std::get_if<GrpcBoolSet>(&property.setter)) {
            propertyObject.setter = std::function<void (bool)>(
                std::bind(*ptr, object, std::placeholders::_1)
                );
        } else if (GrpcDoubleSet * ptr = std::get_if<GrpcDoubleSet>(&property.setter)) {
            propertyObject.setter = std::function<void (double)>(
                std::bind(*ptr, object, std::placeholders::_1)
                );
        }
        // bind getters
        if (GrpcStrGet * ptr = std::get_if<GrpcStrGet>(&property.getter)) {
            propertyObject.getter = std::function<const std::string&()>(
                std::bind(*ptr, object)
                );
        } else if (Grpc64Get * ptr = std::get_if<Grpc64Get>(&property.getter)) {
            propertyObject.getter = std::function<int64_t(void)>(
                std::bind(*ptr, object)
                );
        } else if (Grpc32Get * ptr = std::get_if<Grpc32Get>(&property.getter)) {
            propertyObject.getter = std::function<int32_t(void)>(
                std::bind(*ptr, object)
                );
        } else if (GrpcBoolGet * ptr = std::get_if<GrpcBoolGet>(&property.getter)) {
            propertyObject.getter = std::function<bool(void)>(
                std::bind(*ptr, object)
                );
        } else if (GrpcDoubleGet * ptr = std::get_if<GrpcDoubleGet>(&property.getter)) {
            propertyObject.getter = std::function<double(void)>(
                std::bind(*ptr, object)
                );
        }
        propertiesList.push_back(propertyObject);
    }
    return propertiesList;
}
}

namespace FrontConverter {

QString to_str(const std::string & source);
std::string to_str(const QString & source);
std::string to_str(const QVariant & source);

QVariant to_qvariant_get(GrpcVariantGet varData);
}

namespace GlobalRoles {
enum {
    VariantObjectRole = Qt::UserRole + 1000
    };
}






