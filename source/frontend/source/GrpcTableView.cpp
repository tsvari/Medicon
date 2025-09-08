#include "GrpcTableView.h"
#include <QDebug>

GrpcTableView::GrpcTableView(QWidget * parent)
    : QTableView(parent)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setContextMenuPolicy(Qt::CustomContextMenu);
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
