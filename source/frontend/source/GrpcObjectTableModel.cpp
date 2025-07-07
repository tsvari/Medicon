#include "GrpcObjectTableModel.h"
#include "include_frontend_util.h"

GrpcObjectTableModel::GrpcObjectTableModel(IBaseDataController * controller, QObject *parent)
    : QAbstractTableModel(parent)
    , m_controler(controller)
{
    assert(m_controler==nullptr);
}

QVariant GrpcObjectTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_controler->horizontalHeaderData(section);
    }
    // FIXME: Implement me!
    return QVariant();
}

int GrpcObjectTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

int GrpcObjectTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_controler->propertyCount();
}

QVariant GrpcObjectTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(role == Qt::DisplayRole) {
        return m_controler->data(index.row(), index.column());
    }

    return QVariant();
}

bool GrpcObjectTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        m_controler->setData(index.row(), index.column(), value);
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags GrpcObjectTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}

bool GrpcObjectTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool GrpcObjectTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}

