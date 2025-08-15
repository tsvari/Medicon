#include "GrpcUiTemplate.h"

#include <QTableView>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"

GrpcUiTemplate::GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QTableView * table, GrpcForm * form, QObject *parent)
    : QObject{parent}
    , m_proxyModel(proxyModel)
    , m_table(table)
    , m_form(form)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(table);
    Q_ASSERT(form);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeData();
    form->initializeData();
    table->setModel(proxyModel);

    connect(this, &GrpcUiTemplate::sendObject, form, &GrpcForm::initializeWrapper);
    connect(table->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcUiTemplate::currentChanged);
}

void GrpcUiTemplate::currentChanged(const QModelIndex &, const QModelIndex &)
{
    const QModelIndex & currentIndex = m_table->currentIndex();
    QVariant variantObject = currentIndex.data(GlobalRoles::VariantObjectRole);
    if(variantObject.isValid()) {
        emit sendObject(variantObject);
    }
}
