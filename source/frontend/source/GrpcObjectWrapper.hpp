#ifndef GRPCOBJECT_HPP
#define GRPCOBJECT_HPP

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>
#include <cassert>

struct IBaseGrpcObjectWrapper
{
    virtual ~IBaseGrpcObjectWrapper() = default;

    virtual QVariant data(int col) const = 0;
    virtual GrpcVariantGet nativeData(int col) const = 0;

    virtual void setData(int col, const QVariant & data) = 0;
    virtual void setData(int col, const GrpcVariantSet & data) = 0;

    virtual QVariant variantObject() const = 0;

    virtual int propertyCount() const = 0;
    virtual QVariant propertyWidgetName(int col) const = 0;
    virtual DataInfo::Type dataType(int col) const = 0;

    virtual void setObject(const QVariant & data) = 0;
    virtual bool hasObject() const = 0;

    virtual int dataMask(int col) const = 0;
    virtual QVariant trueData(int col) const = 0;
    virtual QVariant falseData(int col) const = 0;
};


template<typename GrpcObject>
class GrpcObjectWrapper : public IBaseGrpcObjectWrapper
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

    GrpcObjectWrapper() = default;

    ~GrpcObjectWrapper() override = default;

    // Delete copy operations
    GrpcObjectWrapper(const GrpcObjectWrapper&) = delete;
    GrpcObjectWrapper& operator=(const GrpcObjectWrapper&) = delete;

    // Move operations
    GrpcObjectWrapper(GrpcObjectWrapper&&) noexcept = default;
    GrpcObjectWrapper& operator=(GrpcObjectWrapper&&) noexcept = default;

    QVariant propertyWidgetName(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return FrontConverter::to_str(m_properties[col].name);
    }

    int dataMask(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return m_properties[col].dataMask;
    }

    QVariant trueData(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return FrontConverter::to_str(m_properties[col].trueData);
    }

    QVariant falseData(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return FrontConverter::to_str(m_properties[col].falseData);
    }

    int propertyCount() const override {
        return static_cast<int>(m_properties.size());
    }

    QVariant data(int col) const override {
        GrpcVariantGet varData = nativeData(col);
        return FrontConverter::to_qvariant_get(varData);
    }

    GrpcVariantGet nativeData(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_propertyHolders.size()));
        const PropertyHolder& property = m_propertyHolders[col];
        return std::visit([](const auto& getterFunction) -> GrpcVariantGet {
            return getterFunction();
        }, property.getter);
    }

    void setData(int col, const GrpcVariantSet& data) override {
        if (!hasObject()) {
            return;
        }
        assert(col >= 0 && col < static_cast<int>(m_propertyHolders.size()));
        
        PropertyHolder& property = m_propertyHolders[col];
        std::visit(overload{
            [](std::function<void(const std::string&)>& setter, const std::string& param) {
                setter(param);
            },
            [](std::function<void(int64_t)>& setter, int64_t param) {
                setter(param);
            },
            [](std::function<void(int32_t)>& setter, int32_t param) {
                setter(param);
            },
            [](std::function<void(bool)>& setter, bool param) {
                setter(param);
            },
            [](std::function<void(double)>& setter, double param) {
                setter(param);
            },
            [](auto&, auto) {
                // Handle incompatible type combinations
            }
        }, property.setter, data);
    }

    void setData(int col, const QVariant& data) override {
        if (!hasObject()) {
            return;
        }
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        
        GrpcVariantSet variantData = convertQVariantToGrpcVariant(data, m_properties[col].type);
        setData(col, variantData);
    }

    bool hasObject() const override {
        return grpcObject.has_value();
    }

    void setObject(const QVariant & data) override
    {
        grpcObject = std::move(data.value<GrpcObject>());
        bindSettersGetters();
    }
    //////////////////////////////////////////////////
    /// \brief addProperty - template method to reduce code duplication
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    /// \param dataMask
    /// \param trueData
    /// \param falseData
    ///
    template<typename SetterType, typename GetterType>
    void addProperty(const char* name,
                     DataInfo::Type type,
                     SetterType setter,
                     GetterType getter,
                     int dataMask = DataMask::NoMask,
                     const std::string& trueData = "",
                     const std::string& falseData = "")
    {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        property.dataMask = dataMask;
        property.trueData = trueData;
        property.falseData = falseData;
        m_properties.push_back(property);
    }

    DataInfo::Type dataType(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return m_properties[col].type;
    }

    //////////////////////////////////////////////////
    /// \brief variantObject
    /// \return
    ///
    QVariant variantObject() const override {
        if (grpcObject.has_value()) {
            return QVariant::fromValue(*grpcObject);
        }
        return QVariant();
    }

private:
    // Convert QVariant to GrpcVariantSet based on type
    GrpcVariantSet convertQVariantToGrpcVariant(const QVariant& data, DataInfo::Type type) const {
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
    void bindSettersGetters()
    {
        m_propertyHolders.clear();
        for(auto & property: m_properties) {
            PropertyHolder propertyObject{};
            // bind setters
            if (GrpcStrSet * ptr = std::get_if<GrpcStrSet>(&property.setter)) {
                propertyObject.setter = std::function<void (const std::string &)>(
                    std::bind(*ptr,  &(*grpcObject), std::placeholders::_1)
                    );
            } else if (Grpc64Set * ptr = std::get_if<Grpc64Set>(&property.setter)) {
                propertyObject.setter = std::function<void (int64_t)>(
                    std::bind(*ptr,  &(*grpcObject), std::placeholders::_1)
                    );
            } else if (Grpc32Set * ptr = std::get_if<Grpc32Set>(&property.setter)) {
                propertyObject.setter = std::function<void (int32_t)>(
                    std::bind(*ptr, &(*grpcObject), std::placeholders::_1)
                    );
            } else if (GrpcBoolSet * ptr = std::get_if<GrpcBoolSet>(&property.setter)) {
                propertyObject.setter = std::function<void (bool)>(
                    std::bind(*ptr, &(*grpcObject), std::placeholders::_1)
                    );
            } else if (GrpcDoubleSet * ptr = std::get_if<GrpcDoubleSet>(&property.setter)) {
                propertyObject.setter = std::function<void (double)>(
                    std::bind(*ptr, &(*grpcObject), std::placeholders::_1)
                    );
            }
            // bind getters
            if (GrpcStrGet * ptr = std::get_if<GrpcStrGet>(&property.getter)) {
                propertyObject.getter = std::function<const std::string&()>(
                    std::bind(*ptr, &(*grpcObject))
                    );
            } else if (Grpc64Get * ptr = std::get_if<Grpc64Get>(&property.getter)) {
                propertyObject.getter = std::function<int64_t(void)>(
                    std::bind(*ptr, &(*grpcObject))
                    );
            } else if (Grpc32Get * ptr = std::get_if<Grpc32Get>(&property.getter)) {
                propertyObject.getter = std::function<int32_t(void)>(
                    std::bind(*ptr, &(*grpcObject))
                    );
            } else if (GrpcBoolGet * ptr = std::get_if<GrpcBoolGet>(&property.getter)) {
                propertyObject.getter = std::function<bool(void)>(
                    std::bind(*ptr, &(*grpcObject))
                    );
            } else if (GrpcDoubleGet * ptr = std::get_if<GrpcDoubleGet>(&property.getter)) {
                propertyObject.getter = std::function<double(void)>(
                    std::bind(*ptr, &(*grpcObject))
                    );
            }
            m_propertyHolders.push_back(propertyObject);
        }
    }
private:
    std::optional<GrpcObject> grpcObject;
    std::vector<Property<GrpcObject>> m_properties;
    std::vector<PropertyHolder> m_propertyHolders;
};

#endif // GRPCOBJECT_HPP
