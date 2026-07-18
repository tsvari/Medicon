#ifndef GRPCOBJECTTABLEMODEL_H
#define GRPCOBJECTTABLEMODEL_H

#include <QAbstractTableModel>
#include <memory>

class IBaseDataContainer;

/**
 * @brief The GrpcObjectTableModel class provides a table model for GRPC objects
 * 
 * This class wraps an IBaseDataContainer and provides a Qt table model interface
 * for displaying and editing GRPC objects in table views.
 * 
 * @note Thread-safety: This class is NOT thread-safe. All operations must be
 *       performed from the same thread (typically the GUI thread).
 * 
 * @note Lifecycle: The model takes ownership of the container passed to constructor.
 *       Derived classes must call initializeModel() to set up properties before use.
 * 
 * @warning Do not modify the container directly after passing it to the model.
 *          Use the model's methods instead.
 * 
 * Usage example:
 * @code
 * class MyModel : public GrpcObjectTableModel {
 *     void initializeModel() override {
 *         auto* container = dynamic_cast<GrpcDataContainer<MyObject>*>(objectContainer());
 *         container->addProperty("Name", DataInfo::String, &MyObject::set_name, &MyObject::name);
 *         container->addProperty("Age", DataInfo::Int, &MyObject::set_age, &MyObject::age);
 *     }
 * };
 * 
 * MyModel model(new GrpcDataContainer<MyObject>(std::move(data)));
 * model.initializeModel();
 * model.initializeContainer();
 * @endcode
 */
class GrpcObjectTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a GrpcObjectTableModel with the given container
     * @param container The data container (ownership transferred to model)
     * @param parent The parent QObject
     */
    explicit GrpcObjectTableModel(IBaseDataContainer * container, QObject *parent = nullptr);
    
    /**
     * @brief Destructor
     */
    virtual ~GrpcObjectTableModel();

    /**
     * @brief Returns the header data for the given section
     * @param section The section (row or column) number
     * @param orientation Horizontal or vertical
     * @param role The data role (DisplayRole or TextAlignmentRole supported)
     * @return The header data as QVariant
     * 
     * @note For horizontal headers, returns property names.
     *       For vertical headers, returns row numbers (1-based).
     */
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;


    /**
     * @brief Returns the number of rows in the model
     * @param parent The parent index (should be invalid for table models)
     * @return The number of rows
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    
    /**
     * @brief Returns the number of columns in the model
     * @param parent The parent index (should be invalid for table models)
     * @return The number of columns (properties)
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns the data for the given index and role
     * @param index The model index
     * @param role The data role (DisplayRole, TextAlignmentRole, or VariantObjectRole)
     * @return The data as QVariant
     * 
     * @note Supports:
     *       - Qt::DisplayRole: Formatted data for display
     *       - Qt::TextAlignmentRole: Alignment based on data type
     *       - GlobalRoles::VariantObjectRole: Complete object as QVariant
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Sets the data for the given index
     * @param index The model index
     * @param value The new value
     * @param role The data role (only EditRole supported)
     * @return true if data was changed, false if value was the same
     * 
     * @note Emits dataChanged() signal when data is modified
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    /**
     * @brief Returns the item flags for the given index
     * @param index The model index
     * @return The item flags (includes ItemIsEditable)
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief Returns the object at the given row as QVariant
     * @param row The row number
     * @return The object as QVariant
     * @throws std::out_of_range if row is invalid
     */
    QVariant variantObject(int row) const;

public slots:
    /**
     * @brief Inserts an object at the specified row
     * @param row The row index where object should be inserted (0 <= row <= rowCount())
     * @param data The object to insert as QVariant
     * @throws std::out_of_range if row is out of bounds
     * @throws std::invalid_argument if data is invalid or cannot be converted
     * 
     * @note Emits inserted(row) signal on success
     */
    void insertObject(int row, const QVariant & data);
    
    /**
     * @brief Adds a new object at the end of the model
     * @param data The object to add as QVariant
     * @throws std::invalid_argument if data is invalid or cannot be converted
     * 
     * @note Equivalent to insertObject(rowCount(), data)
     * @note Emits inserted(row) signal on success
     */
    void addNewObject(const QVariant & data);
    
    /**
     * @brief Updates the object at the specified row
     * @param row The row index to update (0 <= row < rowCount())
     * @param data The new object data as QVariant
     * @throws std::out_of_range if row is out of bounds
     * @throws std::invalid_argument if data is invalid or cannot be converted
     * 
     * @note Emits updated(row) signal and dataChanged() for entire row
     */
    void updateObject(int row, const QVariant & data);
    
    /**
     * @brief Deletes the object at the specified row
     * @param row The row index to delete (0 <= row < rowCount())
     * @throws std::out_of_range if row is out of bounds
     * 
     * @note Emits deleted(row) signal on success
     * @note Emits zeroCount(true) if this was the last row
     */
    void deleteObject(int row);

signals:
    /**
     * @brief Emitted after a row is successfully inserted
     * @param row The row that was inserted
     */
    void inserted(int row);
    
    /**
     * @brief Emitted after a row is successfully updated
     * @param row The row that was updated
     */
    void updated(int row);
    
    /**
     * @brief Emitted after a row is successfully deleted
     * @param row The row that was deleted
     */
    void deleted(int row);

    /**
     * @brief Emitted when the model becomes empty
     * @param isEmpty Always true (model is now empty)
     */
    void zeroCount(bool isEmpty);

public slots:
    /**
     * @brief Replaces the model's data with data from another container
     * @param container Shared pointer to the source container
     * 
     * @note Calls beginResetModel()/endResetModel()
     * @note Takes ownership of container's data via move semantics
     * @note Emits zeroCount(true) if container is empty
     */
    void setModelData(std::shared_ptr<IBaseDataContainer> container);

    /**
     * @brief Clears the model by replacing its container data with an empty dataset.
     *
     * The concrete container type is preserved.
     * @note Calls beginResetModel()/endResetModel()
     */
    void clearModelData();

protected:
    friend class GrpcTemplateController;
    
    /**
     * @brief Pure virtual method to initialize the model's properties
     * 
     * Derived classes must override this to add properties to the container
     * using container->addProperty() calls.
     * 
     * @note Must be called before initializeContainer()
     */
    virtual void initializeModel() = 0;
    
    /**
     * @brief Returns alignment for the given data type
     * @param type The DataInfo::Type
     * @return QVariant containing Qt::Alignment
     * 
     * @note Can be overridden in derived classes for custom alignment
     */
    QVariant alignment(int type) const;
    
    /**
     * @brief Initializes the container by binding properties
     * 
     * @note Must be called after initializeModel() and before using the model
     */
    void initializeContainer();
    
    /**
     * @brief Returns pointer to the underlying data container
     * @return Pointer to IBaseDataContainer
     */
    IBaseDataContainer * objectContainer() {return m_container.get();}

    /**
     * @brief Protected wrapper for QAbstractTableModel::insertRow
     * @param row The row index
     * @param parent The parent index
     * @return true on success
     * 
     * @note Protected to prevent direct external use
     */
    bool insertRow(int row, const QModelIndex &parent = QModelIndex()) {
        return QAbstractTableModel::insertRow(row, parent);
    }
    
    /**
     * @brief Protected wrapper for QAbstractTableModel::removeRow
     * @param row The row index
     * @param parent The parent index
     * @return true on success
     * 
     * @note Protected to prevent direct external use
     */
    bool removeRow(int row, const QModelIndex &parent = QModelIndex()) {
        return QAbstractTableModel::removeRow(row, parent);
    }

    /**
     * @brief Inserts multiple rows (called by insertRow)
     * @param row Starting row index
     * @param count Number of rows to insert
     * @param parent The parent index
     * @return true on success
     */
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    
    /**
     * @brief Removes multiple rows (called by removeRow)
     * @param row Starting row index
     * @param count Number of rows to remove
     * @param parent The parent index
     * @return true on success
     */
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    /** @brief The underlying data container (owned by model) */
    std::unique_ptr<IBaseDataContainer> m_container;
};

#endif // GRPCOBJECTTABLEMODEL_H
