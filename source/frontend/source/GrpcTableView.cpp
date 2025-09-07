#include "GrpcTableView.h"
#include <QDebug>

GrpcTableView::GrpcTableView(QWidget * parent)
    : QTableView(parent)
{
}

void GrpcTableView::focusInEvent(QFocusEvent * event)
{
    emit focusIn();
    qDebug()<<"Focus In"<<this->objectName();
}

void GrpcTableView::focusOutEvent(QFocusEvent * event)
{
    emit focusOut();
    qDebug()<<"Focus Out"<<this->objectName();
}
