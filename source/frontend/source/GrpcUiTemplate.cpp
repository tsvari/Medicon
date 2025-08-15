#include "GrpcUiTemplate.h"

#include <QTableView>

#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcProxySortFilterModel.h"

GrpcUiTemplate::GrpcUiTemplate(GrpcProxySortFilterModel * proxyModel, QTableView * tableView, GrpcForm * form, QObject *parent)
    : QObject{parent}
    , m_proxyModel(proxyModel)
    , m_tableView(tableView)
    , m_form(form)
{
    Q_ASSERT(proxyModel);
    Q_ASSERT(tableView);
    Q_ASSERT(form);

    GrpcObjectTableModel * sourceModel = qobject_cast<GrpcObjectTableModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(sourceModel);

    sourceModel->initializeData();
    form->initializeData();
    tableView->setModel(proxyModel);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(this, &GrpcUiTemplate::rowChanged, form, &GrpcForm::initializeWrapper);
    connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &GrpcUiTemplate::currentChanged);
}

void GrpcUiTemplate::masterRowChanged(const QModelIndex & index)
{
    emit rowChanged(index);
}

void GrpcUiTemplate::currentChanged(const QModelIndex & current, const QModelIndex & previous)
{
    if(current.row() != previous.row()) {
        emit rowChanged(current);
    }
}
