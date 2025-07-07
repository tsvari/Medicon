#ifndef GRPCDATACONTROLER_H
#define GRPCDATACONTROLER_H

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>

namespace {
using GrpcVariantGet = std::variant<int32_t, std::reference_wrapper<const std::string>, int64_t, bool, double>;
using GrpcVariantSet = std::variant<int32_t, std::string, int64_t, bool, double>;
}
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct IBaseDataController {
    virtual int propertyCount() = 0;
    virtual QVariant data(int row, int col) = 0;
    virtual void setData(int row, int col, const QVariant & data) = 0;
    virtual QVariant horizontalHeaderData(int col) = 0;
    virtual DataInfo::Type dataType(int col) = 0;
};

template<typename GrpcObject>
class GrpcDataController : public IBaseDataController
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

    struct GrpcObjectProperty
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

    struct PropertyMatrix
    {
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

    GrpcDataController(std::vector<GrpcObject> && data) : m_data(std::move(data))
    {
        m_properties.resize(m_data.size());
    }

    // Dont modify aggregated object from outside
    const GrpcObject & object(int row) const {
        return m_data[row];
    }

    // Return copy only
    GrpcObject object(int row) {
        return m_data[row];
    }

    int propertyCount() override {
        return m_data.size();
    }

    QVariant horizontalHeaderData(int col) override {
        return FrontConverter::to_str(m_propertiesMatrix[col].name);
    }

    QVariant data(int row, int col) override {
        GrpcVariantGet varData;
        GrpcObjectProperty property = m_properties[row].at(col);
        varData = std::visit([](const auto & getterFunction) {
            GrpcVariantGet dataToReturn = getterFunction();
            return dataToReturn;
        }, property.getter);

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
        }
        return QVariant();
    }


    void setData(int row, int col, const QVariant & data) override {
        GrpcVariantSet variantData;
        DataInfo::Type type = m_propertiesMatrix.at(col).type;
        std::string propertyName =m_propertiesMatrix.at(col).name;
        switch(type) {
        case DataInfo::Int:
            if(data.canConvert<int32_t>()) {
                variantData = (int32_t)data.toInt();
            }
            break;
        case DataInfo::Int64:
            if(data.canConvert<int64_t>()) {
                variantData = (int64_t)data.toInt();
            }
            break;
        case DataInfo::String:
            if(data.canConvert<QString>()) {
                variantData = FrontConverter::to_str(data.toString());
            }
            break;
        case DataInfo::Double:
            if(data.canConvert<double>()) {
                variantData = (double)data.toInt();
            }
            break;
        case DataInfo::Bool:
            if(data.canConvert<bool>()) {
                variantData = (bool)data.toInt();
            }
            break;
        case DataInfo::Date:
        case DataInfo::DateTime:
        case DataInfo::DateTimeNoSec:
            if(data.canConvert<int64_t>()) {
                variantData = (int64_t)data.toInt();
            }
            break;
        default:
            break;
        }
        GrpcObjectProperty property = m_properties[row].at(col);
        std::visit(overload {
            [](std::function<void(const std::string&)> setter, const std::string & param) {
                           setter(param);
                       },
            [](std::function<void(int64_t)> setter, int64_t param) {
                           setter(param);
                       },
            [](std::function<void(int32_t)> setter, int32_t param) {
                           setter(param);
                       },
            [](std::function<void(bool)> setter, bool param) { setter(param); },
            [](std::function<void(double)> setter, double param) { setter(param); },
            [](auto, auto) { /* Handle other combinations if needed */ }
        }, property.setter, variantData);
    }

    DataInfo::Type dataType(int col) override {
        return m_propertiesMatrix.at(col).type;
    }

    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcStrSet setter,
                     GrpcStrGet getter)
    {
        PropertyMatrix property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_propertiesMatrix.push_back(property);
    }

    void addProperty(const char * name,
                     DataInfo::Type type,
                     Grpc64Set setter,
                     Grpc64Get getter)
    {
        PropertyMatrix property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_propertiesMatrix.push_back(property);
    }

    void addProperty(const char * name,
                     DataInfo::Type type,
                     Grpc32Set setter,
                     Grpc32Get getter)
    {
        PropertyMatrix property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_propertiesMatrix.push_back(property);
    }

    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcBoolSet setter,
                     GrpcBoolGet getter)
    {
        PropertyMatrix property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_propertiesMatrix.push_back(property);
    }

    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcDoubleSet setter,
                     GrpcDoubleGet getter)
    {
        PropertyMatrix property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_propertiesMatrix.push_back(property);
    }

    void initialize()
    {
        int row = 0;
        for(auto & object: m_data) {
            std::vector<GrpcObjectProperty> propertiesList;
            for(auto & property: m_propertiesMatrix) {
                GrpcObjectProperty propertyObject{};
                // bind setters
                if (GrpcStrSet * ptr = std::get_if<GrpcStrSet>(&property.setter)) {
                    propertyObject.setter = std::function<void (const string &)>(
                        std::bind(*ptr,  &object, std::placeholders::_1)
                       );
                } else if (Grpc64Set * ptr = std::get_if<Grpc64Set>(&property.setter)) {
                    propertyObject.setter = std::function<void (int64_t)>(
                        std::bind(*ptr,  &object, std::placeholders::_1)
                        );
                } else if (Grpc32Set * ptr = std::get_if<Grpc32Set>(&property.setter)) {
                    propertyObject.setter = std::function<void (int32_t)>(
                        std::bind(*ptr, &object, std::placeholders::_1)
                        );
                } else if (GrpcBoolSet * ptr = std::get_if<GrpcBoolSet>(&property.setter)) {
                    propertyObject.setter = std::function<void (bool)>(
                        std::bind(*ptr, &object, std::placeholders::_1)
                        );
                } else if (GrpcDoubleSet * ptr = std::get_if<GrpcDoubleSet>(&property.setter)) {
                    propertyObject.setter = std::function<void (double)>(
                        std::bind(*ptr, &object, std::placeholders::_1)
                        );
                }
                // bind getters
                if (GrpcStrGet * ptr = std::get_if<GrpcStrGet>(&property.getter)) {
                    propertyObject.getter = std::function<const std::string&()>(
                        std::bind(*ptr, &object)
                        );
                } else if (Grpc64Get * ptr = std::get_if<Grpc64Get>(&property.getter)) {
                    propertyObject.getter = std::function<int64_t(void)>(
                        std::bind(*ptr, &object)
                        );
                } else if (Grpc32Get * ptr = std::get_if<Grpc32Get>(&property.getter)) {
                    propertyObject.getter = std::function<int32_t(void)>(
                        std::bind(*ptr, &object)
                        );
                } else if (GrpcBoolGet * ptr = std::get_if<GrpcBoolGet>(&property.getter)) {
                    propertyObject.getter = std::function<bool(void)>(
                        std::bind(*ptr, &object)
                        );
                } else if (GrpcDoubleGet * ptr = std::get_if<GrpcDoubleGet>(&property.getter)) {
                    propertyObject.getter = std::function<double(void)>(
                        std::bind(*ptr, &object)
                        );
                }
                propertiesList.push_back(propertyObject);
            }
            m_properties[row] = propertiesList;
            row++;
        }
    }

    using iterator = typename std::vector<GrpcObject>::iterator;
    // Provide begin() and end() methods returning vector iterators
    iterator begin() { return m_data.begin(); }
    iterator end() { return m_data.end(); }

private:
    std::vector<GrpcObject> m_data;
    std::vector<std::vector<GrpcObjectProperty>> m_properties;
    std::vector<PropertyMatrix> m_propertiesMatrix;
};

#endif // GRPCDATACONTROLER_H
