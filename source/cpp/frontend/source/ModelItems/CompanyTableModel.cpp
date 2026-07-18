#include "CompanyTableModel.h"
#include "GrpcDataContainer.hpp"

CompanyTableModel::CompanyTableModel(std::vector<Company> && data, QObject *parent)
    : GrpcObjectTableModel(new GrpcDataContainer<Company>(std::move(data)), parent)
{
    initializeModel();
    initializeContainer();
}

void CompanyTableModel::initializeModel()
{
    GrpcDataContainer<Company> * container = dynamic_cast<GrpcDataContainer<Company>*>(objectContainer());

    container->addProperty("Name", DataInfo::String, &Company::set_name, &Company::name);
    container->addProperty("Address", DataInfo::String, &Company::set_address, &Company::address);
    container->addProperty("Company type", DataInfo::Int, &Company::set_company_type, &Company::company_type);
    container->addProperty("Date", DataInfo::Date, &Company::set_reg_date, &Company::reg_date);
    container->initialize();
}





