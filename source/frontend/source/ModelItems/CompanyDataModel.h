#ifndef COMPANYDATAMODEL_H
#define COMPANYDATAMODEL_H

#include <QAbstractTableModel>

#include "GrpcObjectTableModel.h"
#include "company.grpc.pb.h"

using CompanyEdit::Company;
class CompanyDataModel : public GrpcObjectTableModel
{
    Q_OBJECT

public:
    explicit CompanyDataModel(std::vector<Company> data, QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    GrpcDataController<Company> * controller();
    void initializeData() override;
};

#endif // COMPANYDATAMODEL_H
