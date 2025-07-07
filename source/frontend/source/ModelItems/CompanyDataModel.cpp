#include "CompanyDataModel.h"

CompanyDataModel::CompanyDataModel(std::vector<Company> data, QObject *parent)
    : GrpcObjectTableModel(new GrpcDataController<Company>(std::move(data)), parent)
{
}

GrpcDataController<Company> * CompanyDataModel::controller()
{
    return dynamic_cast<GrpcDataController<Company>*>(m_controler);
}

void CompanyDataModel::initializeData()
{
    controller()->addProperty("Name", DataInfo::String, &Company::set_name, &Company::name);
    controller()->addProperty("Address", DataInfo::String, &Company::set_address, &Company::address);
    controller()->addProperty("Company type", DataInfo::Int, &Company::set_company_type, &Company::company_type);
    controller()->addProperty("Date", DataInfo::Date, &Company::set_reg_date, &Company::reg_date);
    controller()->initialize();
}

QVariant CompanyDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return GrpcObjectTableModel::headerData(section, orientation, role);
}

bool CompanyDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags CompanyDataModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}

bool CompanyDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
    return true;
}

bool CompanyDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
    return true;
}




