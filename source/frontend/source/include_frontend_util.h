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

namespace DataMask {
enum {
    NoMask = 0,
    ComboEditMask,
    CheckBoxMask
};
}

template<typename GrpcObject>
class Property
{
public:
    using Grpc32Set = void (GrpcObject::*)(int32_t);
    using Grpc32Get = int32_t (GrpcObject::*)()const;

    using Grpc64Set = void (GrpcObject::*)(int64_t);
    using Grpc64Get = int64_t (GrpcObject::*)()const;

    using GrpcStrSet = void (GrpcObject::*)(const std::string &);
    using GrpcStrGet = const std::string&(GrpcObject::*)()const;

    using GrpcBoolSet = void (GrpcObject::*)(bool);
    using GrpcBoolGet = bool (GrpcObject::*)()const;

    using GrpcDoubleSet = void (GrpcObject::*)(double);
    using GrpcDoubleGet = double (GrpcObject::*)()const;
    std::string name;  // Header data
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
    int dataMask = DataMask::NoMask;
    std::string trueData = "";
    std::string falseData = "";
};

struct PropertyHolder
{
    std::variant<
        std::function<void (const std::string &)>,
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

    using GrpcStrSet = void (GrpcObject::*)(const std::string &);
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
            propertyObject.setter = std::function<void (const std::string &)>(
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
QVariant to_qvariant_get_by_type(const GrpcVariantGet & varData, DataInfo::Type type);
QVariant to_qvariant_by_type(const QVariant & qVariantData, DataInfo::Type type);
double to_locale_double(const QString & strValue);
}

namespace GlobalRoles {
enum {
    VariantObjectRole = Qt::UserRole + 1000
    };
}

/**
 * @brief Shared helper class for GRPC property binding
 * 
 * This class contains common functionality used by both GrpcObjectWrapper 
 * and GrpcDataContainer to eliminate code duplication (~200 lines).
 * 
 * Provides:
 * - Type aliases for member function pointers
 * - QVariant to GrpcVariant conversion
 * - Property binding logic
 */
template<typename GrpcObject>
class GrpcPropertyBinder
{
public:
    // Type aliases for member function pointers
    using Grpc32Set = void (GrpcObject::*)(int32_t);
    using Grpc32Get = int32_t (GrpcObject::*)() const;

    using Grpc64Set = void (GrpcObject::*)(int64_t);
    using Grpc64Get = int64_t (GrpcObject::*)() const;

    using GrpcStrSet = void (GrpcObject::*)(const std::string &);
    using GrpcStrGet = const std::string& (GrpcObject::*)() const;

    using GrpcBoolSet = void (GrpcObject::*)(bool);
    using GrpcBoolGet = bool (GrpcObject::*)() const;

    using GrpcDoubleSet = void (GrpcObject::*)(double);
    using GrpcDoubleGet = double (GrpcObject::*)() const;

    /**
     * @brief Converts QVariant to GrpcVariantSet based on type
     * @param data QVariant data to convert
     * @param type Target DataInfo::Type
     * @return GrpcVariantSet containing the converted value
     */
    static GrpcVariantSet convertQVariantToGrpcVariant(const QVariant& data, DataInfo::Type type) {
        switch (type) {
            case DataInfo::Int:
                if (data.canConvert<int32_t>()) {
                    return data.toInt();
                }
                break;
            case DataInfo::Int64:
            case DataInfo::Date:
            case DataInfo::DateTime:
            case DataInfo::DateTimeNoSec:
                if (data.canConvert<int64_t>()) {
                    return data.toLongLong();
                }
                break;
            case DataInfo::String:
                if (data.canConvert<QString>()) {
                    return FrontConverter::to_str(data.toString());
                }
                break;
            case DataInfo::Double:
                if (data.canConvert<double>()) {
                    return data.toDouble();
                }
                break;
            case DataInfo::Bool:
                if (data.canConvert<bool>()) {
                    return data.toBool();
                }
                break;
            default:
                break;
        }
        // Return default constructed variant if conversion fails
        return int32_t{0};
    }

    /**
     * @brief Binds setters and getters for a single GRPC object
     * @param object Pointer to the GRPC object
     * @param properties Vector of property definitions
     * @return Vector of PropertyHolder with bound functions
     */
    static std::vector<PropertyHolder> bindSettersGetters(
        GrpcObject* object,
        const std::vector<Property<GrpcObject>>& properties)
    {
        std::vector<PropertyHolder> propertiesList;
        propertiesList.reserve(properties.size());

        for (const auto& property : properties) {
            PropertyHolder propertyObject{};

            // Bind setters using lambdas
            if (const GrpcStrSet* ptr = std::get_if<GrpcStrSet>(&property.setter)) {
                GrpcStrSet setter = *ptr;
                propertyObject.setter = std::function<void(const std::string&)>(
                    [object, setter](const std::string& value) {
                        (object->*setter)(value);
                    });
            } else if (const Grpc64Set* ptr = std::get_if<Grpc64Set>(&property.setter)) {
                Grpc64Set setter = *ptr;
                propertyObject.setter = std::function<void(int64_t)>(
                    [object, setter](int64_t value) {
                        (object->*setter)(value);
                    });
            } else if (const Grpc32Set* ptr = std::get_if<Grpc32Set>(&property.setter)) {
                Grpc32Set setter = *ptr;
                propertyObject.setter = std::function<void(int32_t)>(
                    [object, setter](int32_t value) {
                        (object->*setter)(value);
                    });
            } else if (const GrpcBoolSet* ptr = std::get_if<GrpcBoolSet>(&property.setter)) {
                GrpcBoolSet setter = *ptr;
                propertyObject.setter = std::function<void(bool)>(
                    [object, setter](bool value) {
                        (object->*setter)(value);
                    });
            } else if (const GrpcDoubleSet* ptr = std::get_if<GrpcDoubleSet>(&property.setter)) {
                GrpcDoubleSet setter = *ptr;
                propertyObject.setter = std::function<void(double)>(
                    [object, setter](double value) {
                        (object->*setter)(value);
                    });
            }

            // Bind getters using lambdas
            if (const GrpcStrGet* ptr = std::get_if<GrpcStrGet>(&property.getter)) {
                GrpcStrGet getter = *ptr;
                propertyObject.getter = std::function<const std::string&()>(
                    [object, getter]() -> const std::string& {
                        return (object->*getter)();
                    });
            } else if (const Grpc64Get* ptr = std::get_if<Grpc64Get>(&property.getter)) {
                Grpc64Get getter = *ptr;
                propertyObject.getter = std::function<int64_t()>(
                    [object, getter]() -> int64_t {
                        return (object->*getter)();
                    });
            } else if (const Grpc32Get* ptr = std::get_if<Grpc32Get>(&property.getter)) {
                Grpc32Get getter = *ptr;
                propertyObject.getter = std::function<int32_t()>(
                    [object, getter]() -> int32_t {
                        return (object->*getter)();
                    });
            } else if (const GrpcBoolGet* ptr = std::get_if<GrpcBoolGet>(&property.getter)) {
                GrpcBoolGet getter = *ptr;
                propertyObject.getter = std::function<bool()>(
                    [object, getter]() -> bool {
                        return (object->*getter)();
                    });
            } else if (const GrpcDoubleGet* ptr = std::get_if<GrpcDoubleGet>(&property.getter)) {
                GrpcDoubleGet getter = *ptr;
                propertyObject.getter = std::function<double()>(
                    [object, getter]() -> double {
                        return (object->*getter)();
                    });
            }

            propertiesList.push_back(std::move(propertyObject));
        }
        return propertiesList;
    }
};





