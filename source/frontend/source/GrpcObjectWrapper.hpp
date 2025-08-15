#ifndef GRPCOBJECT_HPP
#define GRPCOBJECT_HPP

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>

struct IBaseGrpcObjectWrapper
{
    virtual ~IBaseGrpcObjectWrapper() = default;

    virtual QVariant data(int col) = 0;

    virtual void setData(int col, const QVariant & data) = 0;
    virtual void setData(int col, const GrpcVariantSet & data) = 0;

    virtual QVariant variantObject() = 0;

    virtual int propertyCount() = 0;
    virtual QVariant propertyWidgetName(int col) = 0;
    virtual DataInfo::Type dataType(int col) = 0;

    virtual void setObject(const QVariant & data) = 0;
    virtual void bindSettersGetters() = 0;
};


template<typename GrpcObject>
class GrpcObjectWrapper : public IBaseGrpcObjectWrapper
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

    GrpcObjectWrapper(){}

    virtual ~GrpcObjectWrapper()
    {
    }

    QVariant propertyWidgetName(int col) override {
        return FrontConverter::to_str(m_properties[col].name);
    }

    int propertyCount() override {
        return m_properties.size();
    }

    QVariant data( int col) override {
        GrpcVariantGet varData;
        PropertyHolder property = m_propertyHolders.at(col);
        varData = std::visit([](const auto & getterFunction) {
            GrpcVariantGet dataToReturn = getterFunction();
            return dataToReturn;
        }, property.getter);
        return FrontConverter::to_qvariant_get(varData);
    }

    void setData(int col, const GrpcVariantSet & data) override {
        PropertyHolder property = m_propertyHolders.at(col);
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
                   }, property.setter, data);
    }

    void setData(int col, const QVariant & data) override {
        GrpcVariantSet variantData;
        DataInfo::Type type = m_properties.at(col).type;
        std::string propertyName =m_properties.at(col).name;
        switch(type) {
        case DataInfo::Int:
            if(data.canConvert<int32_t>()) {
                variantData = data.toInt();
            }
            break;
        case DataInfo::Int64:
            if(data.canConvert<int64_t>()) {
                variantData = data.toLongLong();
            }
            break;
        case DataInfo::String:
            if(data.canConvert<QString>()) {
                variantData = FrontConverter::to_str(data.toString());
            }
            break;
        case DataInfo::Double:
            if(data.canConvert<double>()) {
                variantData = data.toDouble();
            }
            break;
        case DataInfo::Bool:
            if(data.canConvert<bool>()) {
                variantData = data.toBool();
            }
            break;
        case DataInfo::Date:
        case DataInfo::DateTime:
        case DataInfo::DateTimeNoSec:
            if(data.canConvert<int64_t>()) {
                variantData = data.toLongLong();
            }
            break;
        default:
            break;
        }
        setData(col, variantData);
    }

    void setObject(const QVariant & data) override
    {
        grpcObject = grpcObject = std::move(data.value<GrpcObject>());
    }
    //////////////////////////////////////////////////
    /// \brief addProperty
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcStrSet setter,
                     GrpcStrGet getter)
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    //////////////////////////////////////////////////
    /// \brief addProperty
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    void addProperty(const char * name,
                     DataInfo::Type type,
                     Grpc64Set setter,
                     Grpc64Get getter)
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    //////////////////////////////////////////////////
    /// \brief addProperty
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    void addProperty(const char * name,
                     DataInfo::Type type,
                     Grpc32Set setter,
                     Grpc32Get getter)
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    //////////////////////////////////////////////////
    /// \brief addProperty
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcBoolSet setter,
                     GrpcBoolGet getter)
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    //////////////////////////////////////////////////
    /// \brief addProperty
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    void addProperty(const char * name,
                     DataInfo::Type type,
                     GrpcDoubleSet setter,
                     GrpcDoubleGet getter)
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    DataInfo::Type dataType(int col) override {
        return m_properties.at(col).type;
    }

    //////////////////////////////////////////////////
    /// \brief variantObject
    /// \param row
    /// \return
    ///
    QVariant variantObject() override {
        return QVariant::fromValue(grpcObject);
    }

    void bindSettersGetters() override
    {
        m_propertyHolders.clear();
        for(auto & property: m_properties) {
            PropertyHolder propertyObject{};
            // bind setters
            if (GrpcStrSet * ptr = std::get_if<GrpcStrSet>(&property.setter)) {
                propertyObject.setter = std::function<void (const string &)>(
                    std::bind(*ptr,  grpcObject, std::placeholders::_1)
                    );
            } else if (Grpc64Set * ptr = std::get_if<Grpc64Set>(&property.setter)) {
                propertyObject.setter = std::function<void (int64_t)>(
                    std::bind(*ptr,  grpcObject, std::placeholders::_1)
                    );
            } else if (Grpc32Set * ptr = std::get_if<Grpc32Set>(&property.setter)) {
                propertyObject.setter = std::function<void (int32_t)>(
                    std::bind(*ptr, grpcObject, std::placeholders::_1)
                    );
            } else if (GrpcBoolSet * ptr = std::get_if<GrpcBoolSet>(&property.setter)) {
                propertyObject.setter = std::function<void (bool)>(
                    std::bind(*ptr, grpcObject, std::placeholders::_1)
                    );
            } else if (GrpcDoubleSet * ptr = std::get_if<GrpcDoubleSet>(&property.setter)) {
                propertyObject.setter = std::function<void (double)>(
                    std::bind(*ptr, grpcObject, std::placeholders::_1)
                    );
            }
            // bind getters
            if (GrpcStrGet * ptr = std::get_if<GrpcStrGet>(&property.getter)) {
                propertyObject.getter = std::function<const std::string&()>(
                    std::bind(*ptr, grpcObject)
                    );
            } else if (Grpc64Get * ptr = std::get_if<Grpc64Get>(&property.getter)) {
                propertyObject.getter = std::function<int64_t(void)>(
                    std::bind(*ptr, grpcObject)
                    );
            } else if (Grpc32Get * ptr = std::get_if<Grpc32Get>(&property.getter)) {
                propertyObject.getter = std::function<int32_t(void)>(
                    std::bind(*ptr, grpcObject)
                    );
            } else if (GrpcBoolGet * ptr = std::get_if<GrpcBoolGet>(&property.getter)) {
                propertyObject.getter = std::function<bool(void)>(
                    std::bind(*ptr, grpcObject)
                    );
            } else if (GrpcDoubleGet * ptr = std::get_if<GrpcDoubleGet>(&property.getter)) {
                propertyObject.getter = std::function<double(void)>(
                    std::bind(*ptr, grpcObject)
                    );
            }
            m_propertyHolders.push_back(propertyObject);
        }
    }
private:
    GrpcObject grpcObject;
    std::vector<Property<GrpcObject>> m_properties;
    std::vector<PropertyHolder> m_propertyHolders;
};

#endif // GRPCOBJECT_HPP
