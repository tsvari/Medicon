#ifndef GRPCOBJECT_HPP
#define GRPCOBJECT_HPP

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>

struct IBaseGrpcObjectWrapper
{
    virtual ~IBaseGrpcObjectWrapper() = default;

    virtual QVariant variantObject(int row) = 0;
};


template<typename GrpcObject>
class GrpcObjectWrapper : public IBaseGrpcObjectWrapper
{
public:
    GrpcObject grpcObject;

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

    GrpcObjectWrapper(const std::vector<Property<GrpcObject>> & properties)
        : m_properties(properties)
    {

    }

    virtual ~GrpcObjectWrapper()
    {
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


    //////////////////////////////////////////////////
    /// \brief variantObject
    /// \param row
    /// \return
    ///
    QVariant variantObject(int row) override {
        return QVariant::fromValue(grpcObject);
    }

public:
    void bindSettersGetters()
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

    std::vector<Property<GrpcObject>> m_properties;
    std::vector<PropertyHolder> m_propertyHolders;
};

#endif // GRPCOBJECT_HPP
