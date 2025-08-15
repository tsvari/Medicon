#ifndef GRPCDATACONTROLER_H
#define GRPCDATACONTROLER_H

#include "include_frontend_util.h"
#include <functional>
#include <QVariant>

struct IBaseDataContainer {
    virtual ~IBaseDataContainer() = default;

    virtual int propertyCount() = 0;
    virtual int count() = 0;

    virtual QVariant data(int row, int col) = 0;
    virtual GrpcVariantGet nativeData(int row, int col) = 0;
    virtual QVariant horizontalHeaderData(int col) = 0;

    virtual void setData(int row, int col, const QVariant & data) = 0;
    virtual void setData(int row, int col, const GrpcVariantSet & data) = 0;

    virtual DataInfo::Type dataType(int col) = 0;

    virtual void insertObject(int row, const QVariant & data) = 0;
    virtual void deleteObject(int row) = 0;
    virtual void addNewObject(const QVariant & data) = 0;
    virtual void updateObject(int row, const QVariant & data) = 0;

    virtual void initialize() = 0;

    virtual QVariant variantObject(int row) = 0;
};

template<typename GrpcObject>
class GrpcDataContainer : public IBaseDataContainer
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

    //////////////////////////////////////////////////
    /// \brief GrpcDataContainer
    /// \param data
    ///
    GrpcDataContainer(std::vector<GrpcObject> && data) //: m_data(std::move(data))
    {
        m_data.resize(data.size());
        m_propertyHolders.resize(data.size());
        for(int i = 0; i < data.size(); ++i) {
            m_data[i] = new GrpcObject(std::move(data[i]));
        }

    }

    // Default constructor
    GrpcDataContainer() = default;

    // Destructor
    virtual ~GrpcDataContainer()
    {
        removeAll();
    }


    //////////////////////////////////////////////////
    /// \brief Dont modify aggregated object from outside
    /// \param row
    /// \return
    ///
    const GrpcObject & object(int row) const {
        return *m_data[row];
    }

    //////////////////////////////////////////////////
    /// \brief Return copy only
    /// \param row
    /// \return
    ///
    GrpcObject object(int row) {
        return *m_data[row];
    }

    //////////////////////////////////////////////////
    /// \brief variantObject
    /// \param row
    /// \return
    ///
    QVariant variantObject(int row) override {
        return QVariant::fromValue(object(row));
    }

    //////////////////////////////////////////////////
    /// \brief propertyCount
    /// \return
    ///
    int propertyCount() override {
        return m_properties.size();
    }

    //////////////////////////////////////////////////
    /// \brief count
    /// \return
    ///
    int count() override {
        return m_data.size();
    }

    //////////////////////////////////////////////////
    /// \brief horizontalHeaderData
    /// \param col
    /// \return
    ///
    QVariant horizontalHeaderData(int col) override {
        return FrontConverter::to_str(m_properties[col].name);
    }
    ///////////////////////////////////////////////////
    /// \brief data
    /// \param row
    /// \param col
    /// \return
    ///
    QVariant data(int row, int col) override {
        GrpcVariantGet varData = nativeData(row, col);
        return FrontConverter::to_qvariant_get(varData);
    }

    //////////////////////////////////////////////////
    /// \brief nativeData
    /// \param row
    /// \param col
    /// \return
    ///
    GrpcVariantGet nativeData(int row, int col) override {
        GrpcVariantGet varData;
        PropertyHolder property = m_propertyHolders[row].at(col);
        varData = std::visit([](const auto & getterFunction) {
            GrpcVariantGet dataToReturn = getterFunction();
            return dataToReturn;
        }, property.getter);
        return varData; // it's safe because of object is alocated
    }

    //////////////////////////////////////////////////
    /// \brief setData
    /// \param row
    /// \param col
    /// \param data
    ///
    void setData(int row, int col, const GrpcVariantSet & data) override {
        PropertyHolder property = m_propertyHolders[row].at(col);
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

    //////////////////////////////////////////////////
    /// \brief setData
    /// \param row
    /// \param col
    /// \param data
    ///
    void setData(int row, int col, const QVariant & data) override {
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
        setData(row, col, variantData);
    }

    //////////////////////////////////////////////////
    /// \brief dataType
    /// \param col
    /// \return
    ///
    DataInfo::Type dataType(int col) override {
        return m_properties.at(col).type;
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
    /// \brief Bind setters and getters from PropertyHolder object to GrpcObject from m_data list
    ///
    void initialize() override
    {
        int row = 0;
        for(GrpcObject * object: m_data) {
            std::vector<PropertyHolder> propertiesList = std::move(bindSettersGetters(object));
            m_propertyHolders[row] = propertiesList;
            row++;
        }
    }

    void insertObject(int row, const QVariant & data) override
    {
        if(data.isValid()) {
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
        if(data.isValid()) {
            GrpcObject object = data.value<GrpcObject>();
            update(row, object);
        }
    }

    void addNewObject(const QVariant & data) override
    {
        if(data.isValid()) {
            GrpcObject object = data.value<GrpcObject>();
            addNew(object);
        }
    }

    void insert(int row, GrpcObject & object)
    {
        m_data.insert(m_data.begin() + row, new GrpcObject(object));
        m_propertyHolders.insert(m_propertyHolders.begin() + row, bindSettersGetters(m_data[row]));
    }

    void update(int row, GrpcObject & object)
    {
        delete m_data[row];
        m_data[row] = new GrpcObject(object);
        m_propertyHolders[row] = bindSettersGetters(m_data[row]);
    }

    void addNew(GrpcObject & object)
    {
        m_data.push_back(new GrpcObject(object));
        m_propertyHolders.push_back(bindSettersGetters(m_data[m_data.size() - 1]));
    }

    void remove(int row)
    {
        delete m_data[row];
        m_data.erase(m_data.begin() + row);
        m_propertyHolders.erase(m_propertyHolders.begin() + row);
    }

    void removeAll()
    {
        for (auto it = m_data.begin(); it != m_data.end(); ) {
            delete *it; // Delete the object pointed to by the pointer
            it = m_data.erase(it); // Remove the pointer from the vector and update iterator
        }
        m_data.clear();
        m_propertyHolders.clear();
    }

private:
    std::vector<PropertyHolder> bindSettersGetters(GrpcObject * object)
    {
        std::vector<PropertyHolder> propertiesList;
        for(auto & property: m_properties) {
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
    //using iterator = typename std::vector<GrpcObject>::iterator;
    //// Provide begin() and end() methods returning vector iterators
    //iterator begin() { return m_data.begin(); }
    //iterator end() { return m_data.end(); }

private:
    std::vector<GrpcObject*> m_data;
    std::vector<std::vector<PropertyHolder>> m_propertyHolders;
    std::vector<Property<GrpcObject>> m_properties;
};

#endif // GRPCDATACONTROLER_H
