#include "GrpcTableView.h"
#include <QDebug>
#include <QHeaderView>
#include <QMessageBox>

GrpcTableView::GrpcTableView(QWidget * parent)
    : QTableView(parent)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContentsOnFirstShow);
    setHorizontalScrollMode(ScrollPerPixel);
    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void GrpcTableView::select(int row)
{
    clearRowSelection();

    scrollToBottom();
    QModelIndex indexToSelect = model()->index(row, 0);
    selectionModel()->select(indexToSelect, QItemSelectionModel::Select | QItemSelectionModel::Rows);
}

void GrpcTableView::clearRowSelection()
{
    clearSelection();
    setCurrentIndex(QModelIndex());
}

void GrpcTableView::showWarning(const QString &warningTitle, const QString &message)
{
    QMessageBox::warning(this,  warningTitle,  message);
}

void GrpcTableView::focusInEvent(QFocusEvent * event)
{
    emit focusIn();
    //qDebug()<<"Focus In"<<this->objectName();
    QTableView::focusInEvent(event);
}

void GrpcTableView::focusOutEvent(QFocusEvent * event)
{
    emit focusOut();
    //qDebug()<<"Focus Out"<<this->objectName();
    QTableView::focusInEvent(event);
}

