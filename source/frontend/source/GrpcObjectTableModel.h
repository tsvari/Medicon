#ifndef GRPCOBJECTTABLEMODEL_H
#define GRPCOBJECTTABLEMODEL_H

#include <QAbstractTableModel>

class IBaseDataContainer;
class GrpcObjectTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GrpcObjectTableModel(IBaseDataContainer * container, QObject *parent = nullptr);
    virtual ~GrpcObjectTableModel();

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;


    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant variantObject(int row);

public slots:
    void insertObject(int row, const QVariant & data);
    void addNewObject(const QVariant & data);
    void updateObject(int row, const QVariant & data);
    void deleteObject(int row);

signals:
    void inserted(int row);
    void updated(int row);
    void deleted(int row);

    void zerroCount(bool isEmpty);

public slots:
    void setModelData(std::shared_ptr<IBaseDataContainer> container);

protected:
    friend class GrpcTemplateController;
    // be sure to override it in the child
    virtual void initializeModel() = 0;
    void initializeContainer();
    IBaseDataContainer * objectContainer() {return m_container.get();}

    // Just protect it from being explicitly used by the object.
    bool insertRow(int row, const QModelIndex &parent = QModelIndex()) {
        return QAbstractTableModel::insertRow(row, parent);
    }
    bool removeRow(int row, const QModelIndex &parent = QModelIndex()) {
        return QAbstractTableModel::removeRow(row, parent);
    }

    // Just protect it from being explicitly used by the object.
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    QVariant alignment(int type) const;

    std::unique_ptr<IBaseDataContainer> m_container; // delete or smart pointer
};

#endif // GRPCOBJECTTABLEMODEL_H
