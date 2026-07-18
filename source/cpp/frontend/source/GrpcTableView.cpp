#include "GrpcTableView.h"

#include <QAction>
#include <QDebug>
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

    m_actionEscape = new QAction(tr("Escape Action"), this);
    m_actionEscape->setObjectName("actionEscape");
    m_actionEscape->setShortcut(QKeySequence(Qt::Key_Escape));
    m_actionEscape->setShortcutContext(Qt::WidgetShortcut);
    addAction(m_actionEscape);
    connect(m_actionEscape, &QAction::triggered, this, [this] {
        emit escapePressed();
    });
}

GrpcTableView::~GrpcTableView()
{
    if (m_actionEscape) {
        removeAction(m_actionEscape);
    }
}

void GrpcTableView::select(int row)
{
    if (!model() || !selectionModel()) {
        return;
    }

    if (row < 0 || row >= model()->rowCount()) {
        return;
    }

    clearRowSelection();

    const QModelIndex indexToSelect = model()->index(row, 0);
    if (!indexToSelect.isValid()) {
        return;
    }

    selectionModel()->setCurrentIndex(indexToSelect, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    scrollTo(indexToSelect, QAbstractItemView::PositionAtCenter);
}

void GrpcTableView::clearRowSelection()
{
    if (!selectionModel()) {
        return;
    }
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
    QTableView::focusOutEvent(event);
}

