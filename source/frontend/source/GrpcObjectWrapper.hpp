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
    // Use type aliases from shared helper
    using Grpc32Set = typename GrpcPropertyBinder<GrpcObject>::Grpc32Set;
    using Grpc32Get = typename GrpcPropertyBinder<GrpcObject>::Grpc32Get;
    using Grpc64Set = typename GrpcPropertyBinder<GrpcObject>::Grpc64Set;
    using Grpc64Get = typename GrpcPropertyBinder<GrpcObject>::Grpc64Get;
    using GrpcStrSet = typename GrpcPropertyBinder<GrpcObject>::GrpcStrSet;
    using GrpcStrGet = typename GrpcPropertyBinder<GrpcObject>::GrpcStrGet;
    using GrpcBoolSet = typename GrpcPropertyBinder<GrpcObject>::GrpcBoolSet;
    using GrpcBoolGet = typename GrpcPropertyBinder<GrpcObject>::GrpcBoolGet;
    using GrpcDoubleSet = typename GrpcPropertyBinder<GrpcObject>::GrpcDoubleSet;
    using GrpcDoubleGet = typename GrpcPropertyBinder<GrpcObject>::GrpcDoubleGet;

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
        
        // Use shared helper for conversion
        GrpcVariantSet variantData = GrpcPropertyBinder<GrpcObject>::convertQVariantToGrpcVariant(
            data, m_properties[col].type);
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
private:
    void bindSettersGetters()
    {
        if (!grpcObject.has_value()) {
            m_propertyHolders.clear();
            return;
        }
        // Use shared helper to bind properties
        m_propertyHolders = GrpcPropertyBinder<GrpcObject>::bindSettersGetters(
            &(*grpcObject), m_properties);
    }

private:
    std::optional<GrpcObject> grpcObject;
    std::vector<Property<GrpcObject>> m_properties;
    std::vector<PropertyHolder> m_propertyHolders;
};

#endif // GRPCOBJECT_HPP
