#ifndef COMPANYDATAMODEL_H
#define COMPANYDATAMODEL_H

#include <QAbstractTableModel>

#include "GrpcObjectTableModel.h"
#include "company.grpc.pb.h"

using CompanyEdit::Company;
class CompanyTableModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit CompanyTableModel(std::vector<Company> && data, QObject *parent = nullptr);

private:
    GrpcDataContainer<Company> * container();
    void initializeData() override;
};

#endif // COMPANYDATAMODEL_H
