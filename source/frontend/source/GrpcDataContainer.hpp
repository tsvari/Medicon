#ifndef GRPCDATACONTAINER_H
#define GRPCDATACONTAINER_H

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>
#include <memory>
#include <cassert>

struct IBaseDataContainer {
    virtual ~IBaseDataContainer() = default;

    virtual int propertyCount() const = 0;
    virtual int count() const = 0;

    virtual QVariant data(int row, int col) const = 0;
    virtual GrpcVariantGet nativeData(int row, int col) const = 0;
    virtual QVariant horizontalHeaderData(int col) const = 0;

    virtual void setData(int row, int col, const QVariant & data) = 0;
    virtual void setData(int row, int col, const GrpcVariantSet & data) = 0;

    virtual DataInfo::Type dataType(int col) const = 0;

    virtual void insertObject(int row, const QVariant & data) = 0;
    virtual void deleteObject(int row) = 0;
    virtual void addNewObject(const QVariant & data) = 0;
    virtual void updateObject(int row, const QVariant & data) = 0;

    virtual void initialize() = 0;
    virtual void acquireData(IBaseDataContainer * source) = 0;

    virtual QVariant variantObject(int row) const = 0;
};

template<typename GrpcObject>
class GrpcDataContainer : public IBaseDataContainer
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

    //////////////////////////////////////////////////
    /// \brief GrpcDataContainer
    /// \param data
    ///
    explicit GrpcDataContainer(std::vector<GrpcObject> && data) {
        m_data.reserve(data.size());
        m_propertyHolders.reserve(data.size());
        
        for (auto && obj : data) {
            m_data.push_back(std::make_unique<GrpcObject>(std::move(obj)));
        }
        m_propertyHolders.resize(m_data.size());
    }

    // Default constructor
    GrpcDataContainer() = default;

    // Destructor
    ~GrpcDataContainer() override = default;

    // Delete copy operations to prevent accidental copying
    GrpcDataContainer(const GrpcDataContainer&) = delete;
    GrpcDataContainer& operator=(const GrpcDataContainer&) = delete;

    // Move operations
    GrpcDataContainer(GrpcDataContainer&&) noexcept = default;
    GrpcDataContainer& operator=(GrpcDataContainer&&) noexcept = default;

    //////////////////////////////////////////////////
    /// \brief Dont modify aggregated object from outside
    /// \param row
    /// \return
    ///
    const GrpcObject & object(int row) const {
        assert(row >= 0 && row < static_cast<int>(m_data.size()));
        return *m_data[row];
    }

    //////////////////////////////////////////////////
    /// \brief Return copy only
    /// \param row
    /// \return
    ///
    GrpcObject object(int row) {
        assert(row >= 0 && row < static_cast<int>(m_data.size()));
        return *m_data[row];
    }

    //////////////////////////////////////////////////
    /// \brief variantObject
    /// \param row
    /// \return
    ///
    QVariant variantObject(int row) const override {
        return QVariant::fromValue(object(row));
    }

    //////////////////////////////////////////////////
    /// \brief propertyCount
    /// \return
    ///
    int propertyCount() const override {
        return static_cast<int>(m_properties.size());
    }

    //////////////////////////////////////////////////
    /// \brief count
    /// \return
    ///
    int count() const override {
        return static_cast<int>(m_data.size());
    }

    //////////////////////////////////////////////////
    /// \brief horizontalHeaderData
    /// \param col
    /// \return
    ///
    QVariant horizontalHeaderData(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return FrontConverter::to_str(m_properties[col].name);
    }
    ///////////////////////////////////////////////////
    /// \brief data
    /// \param row
    /// \param col
    /// \return
    ///
    QVariant data(int row, int col) const override {
        GrpcVariantGet varData = nativeData(row, col);
        return FrontConverter::to_qvariant_get(varData);
    }

    //////////////////////////////////////////////////
    /// \brief nativeData
    /// \param row
    /// \param col
    /// \return
    ///
    GrpcVariantGet nativeData(int row, int col) const override {
        assert(row >= 0 && row < static_cast<int>(m_propertyHolders.size()));
        assert(col >= 0 && col < static_cast<int>(m_propertyHolders[row].size()));
        
        const PropertyHolder& property = m_propertyHolders[row][col];
        return std::visit([](const auto& getterFunction) -> GrpcVariantGet {
            return getterFunction();
        }, property.getter);
    }

    //////////////////////////////////////////////////
    /// \brief setData
    /// \param row
    /// \param col
    /// \param data
    ///
    void setData(int row, int col, const GrpcVariantSet & data) override {
        assert(row >= 0 && row < static_cast<int>(m_propertyHolders.size()));
        assert(col >= 0 && col < static_cast<int>(m_propertyHolders[row].size()));
        
        PropertyHolder& property = m_propertyHolders[row][col];
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

    //////////////////////////////////////////////////
    /// \brief setData
    /// \param row
    /// \param col
    /// \param data
    ///
    void setData(int row, int col, const QVariant & data) override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        
        // Use shared helper for conversion
        GrpcVariantSet variantData = GrpcPropertyBinder<GrpcObject>::convertQVariantToGrpcVariant(
            data, m_properties[col].type);
        setData(row, col, variantData);
    }

    //////////////////////////////////////////////////
    /// \brief dataType
    /// \param col
    /// \return
    ///
    DataInfo::Type dataType(int col) const override {
        assert(col >= 0 && col < static_cast<int>(m_properties.size()));
        return m_properties[col].type;
    }

    //////////////////////////////////////////////////
    /// \brief addProperty - template method to reduce code duplication
    /// \param name
    /// \param type
    /// \param setter
    /// \param getter
    ///
    template<typename SetterType, typename GetterType>
    void addProperty(const char* name, DataInfo::Type type, SetterType setter, GetterType getter) {
        Property<GrpcObject> property;
        property.name = name;
        property.type = type;
        property.setter = setter;
        property.getter = getter;
        m_properties.push_back(property);
    }

    //////////////////////////////////////////////////
    /// \brief Bind setters and getters from PropertyHolder object to GrpcObject from m_data list
    ///
    void initialize() override
    {
        for (size_t row = 0; row < m_data.size(); ++row) {
            m_propertyHolders[row] = bindSettersGetters(m_data[row].get());
        }
    }

    void insertObject(int row, const QVariant & data) override
    {
        if(data.isValid() && data.canConvert<GrpcObject>()) {
            GrpcObject object = data.value<GrpcObject>();
            insert(row, object);
        }
    }

    void deleteObject(int row) override
    {
        remove(row);
    }

    void updateObject(int row, const QVariant & data) override
    {
        if(data.isValid() && data.canConvert<GrpcObject>()) {
            GrpcObject object = data.value<GrpcObject>();
            update(row, object);
        }
    }

    void addNewObject(const QVariant & data) override
    {
        if(data.isValid() && data.canConvert<GrpcObject>()) {
            GrpcObject object = data.value<GrpcObject>();
            addNew(object);
        }
    }

    void insert(int row, const GrpcObject & object)
    {
        assert(row >= 0 && row <= static_cast<int>(m_data.size()));
        
        auto newObject = std::make_unique<GrpcObject>(object);
        auto newPropertyHolder = bindSettersGetters(newObject.get());
        
        m_data.insert(m_data.begin() + row, std::move(newObject));
        m_propertyHolders.insert(m_propertyHolders.begin() + row, std::move(newPropertyHolder));
    }

    void update(int row, const GrpcObject & object)
    {
        assert(row >= 0 && row < static_cast<int>(m_data.size()));
        
        m_data[row] = std::make_unique<GrpcObject>(object);
        m_propertyHolders[row] = bindSettersGetters(m_data[row].get());
    }

    void addNew(const GrpcObject & object)
    {
        auto newObject = std::make_unique<GrpcObject>(object);
        auto newPropertyHolder = bindSettersGetters(newObject.get());
        
        m_data.push_back(std::move(newObject));
        m_propertyHolders.push_back(std::move(newPropertyHolder));
    }

    void remove(int row)
    {
        assert(row >= 0 && row < static_cast<int>(m_data.size()));
        
        m_data.erase(m_data.begin() + row);
        m_propertyHolders.erase(m_propertyHolders.begin() + row);
    }

    void acquireData(IBaseDataContainer * source) override {
        auto* typedSource = dynamic_cast<GrpcDataContainer*>(source);
        assert(typedSource && "Source must be of type GrpcDataContainer<GrpcObject>");
        
        if (typedSource) {
            m_data = std::move(typedSource->m_data);
            m_propertyHolders.resize(m_data.size());
        }
    }

private:
    // Bind setters and getters using shared helper
    std::vector<PropertyHolder> bindSettersGetters(GrpcObject* object) const
    {
        return GrpcPropertyBinder<GrpcObject>::bindSettersGetters(object, m_properties);
    }

private:
    std::vector<std::unique_ptr<GrpcObject>> m_data;
    std::vector<std::vector<PropertyHolder>> m_propertyHolders;
    std::vector<Property<GrpcObject>> m_properties;
};

#endif // GRPCDATACONTAINER_H
