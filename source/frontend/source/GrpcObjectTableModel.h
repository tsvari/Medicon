#ifndef GRPCOBJECTTABLEMODEL_H
#define GRPCOBJECTTABLEMODEL_H

#include "GrpcDataContainer.hpp"
#include <QAbstractTableModel>

class IBaseDataContainer;
class GrpcObjectTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GrpcObjectTableModel(IBaseDataContainer * container, QObject *parent = nullptr);

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

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    virtual void initializeData() = 0;

protected:
    IBaseDataContainer * m_container = nullptr;
};

#endif // GRPCOBJECTTABLEMODEL_H
