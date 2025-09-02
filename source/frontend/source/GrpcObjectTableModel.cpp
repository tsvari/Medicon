#include "GrpcObjectTableModel.h"
#include "include_frontend_util.h"
#include "GrpcDataContainer.hpp"
#include "GrpcTemplateController.h"


GrpcObjectTableModel::GrpcObjectTableModel(IBaseDataContainer * container, QObject * parent)
    : QAbstractTableModel(parent)
    , m_container(container)
{
}

GrpcObjectTableModel::~GrpcObjectTableModel()
{
}

QVariant GrpcObjectTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_container->horizontalHeaderData(section);
    } else if(orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return section + 1;
    } else if(orientation == Qt::Horizontal && role == Qt::TextAlignmentRole) {
        return alignment(m_container->dataType(section));
    }
    return QVariant();
}

int GrpcObjectTableModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_container->count();
}

int GrpcObjectTableModel::columnCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_container->propertyCount();
}

QVariant GrpcObjectTableModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if(role == Qt::DisplayRole) {
        return m_container->data(index.row(), index.column());
    } else if (role == GlobalRoles::VariantObjectRole) {
        return m_container->variantObject(index.row());
    } else if(role == Qt::TextAlignmentRole) {
        return alignment(m_container->dataType(index.column()));
    }

    return QVariant();
}

bool GrpcObjectTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (data(index, role) != value) {
        m_container->setData(index.row(), index.column(), value);
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

void GrpcObjectTableModel::insertObject(int row, const QVariant & data)
{
    insertRow(row);
    m_container->insertObject(row, data);  
    // Simply emit a signal to move the cursor to the newly inserted row and select it
    emit inserted(row);
}

void GrpcObjectTableModel::addNewObject(const QVariant & data)
{
    insertObject(rowCount(), data);
}

void GrpcObjectTableModel::updateObject(int row, const QVariant &data)
{
    m_container->updateObject(row, data);
    emit updated(row);
}

void GrpcObjectTableModel::deleteObject(int row)
{
    removeRow(row);
    m_container->deleteObject(row);
    emit deleted(row);
}

QVariant GrpcObjectTableModel::variantObject(int row)
{
    return m_container->variantObject(row);
}

void GrpcObjectTableModel::setModelData(std::shared_ptr<IBaseDataContainer> container)
{
    beginResetModel();

    m_container->acquireData(container.get());
    if(m_container->count() == 0) {
        emit zerroCount();
    }
    m_container->initialize();

    endResetModel();
}

void GrpcObjectTableModel::initializeContainer()
{
    m_container->initialize();
}

bool GrpcObjectTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // Do nothing, the new object wil be inserted into m_container
    endInsertRows();

    return true;
}

bool GrpcObjectTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // Do nothing, the new object wil be removed from m_container
    endRemoveRows();
    return true;
}

QVariant GrpcObjectTableModel::alignment(int type) const
{
    switch(static_cast<DataInfo::Type>(type)) {
    case DataInfo::DateTime:
    case DataInfo::DateTimeNoSec:
    case DataInfo::Date:
    case DataInfo::Time:
    case DataInfo::Bool:
        return QVariant::fromValue<Qt::Alignment>(Qt::AlignCenter);
    case DataInfo::Int:
    case DataInfo::Int64:
    case DataInfo::Double:
        return QVariant::fromValue<Qt::Alignment>(Qt::AlignRight | Qt::AlignVCenter);
    default:
        QVariant::fromValue<Qt::Alignment>(Qt::AlignLeft | Qt::AlignVCenter);
    }
    return QVariant();
}

