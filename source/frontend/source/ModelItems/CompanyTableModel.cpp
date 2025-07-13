#include "CompanyTableModel.h"

CompanyTableModel::CompanyTableModel(std::vector<Company> && data, QObject *parent)
    : GrpcObjectTableModel(new GrpcDataContainer<Company>(std::move(data)), parent)
{
}

GrpcDataContainer<Company> * CompanyTableModel::container()
{
    return dynamic_cast<GrpcDataContainer<Company>*>(m_container);
}

void CompanyTableModel::initializeData()
{
    container()->addProperty("Name", DataInfo::String, &Company::set_name, &Company::name);
    container()->addProperty("Address", DataInfo::String, &Company::set_address, &Company::address);
    container()->addProperty("Company type", DataInfo::Int, &Company::set_company_type, &Company::company_type);
    container()->addProperty("Date", DataInfo::Date, &Company::set_reg_date, &Company::reg_date);
    container()->initialize();
}





