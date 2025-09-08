#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcTemplateController.h"

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QTextEdit>

#include <QDebug>

GrpcForm::GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, QWidget *parent)
    : QWidget{parent}
    , m_objectWrapper(objectWrapper)
{
    m_saveIcon = QIcon(":/icons/save.png");
}

GrpcForm::~GrpcForm()
{
}

QVariant GrpcForm::object()
{
    return m_objectWrapper->variantObject();
}

void GrpcForm::fill(const QModelIndex & index)
{
    m_formFillingFinished = false;
    const QVariant varData = index.data(GlobalRoles::VariantObjectRole);
    if(varData.isValid()) {
        m_objectWrapper->setObject(varData);
        Q_ASSERT(m_formWidgets.count() == m_objectWrapper->propertyCount());
        for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
            DataInfo::Type type = m_objectWrapper->dataType(i);
            fillWidget(m_formWidgets[i], type, m_objectWrapper->data(i));
        }
    }
    m_formFillingFinished = true;
}

void GrpcForm::fillObject()
{
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        Q_ASSERT(widget);
        QVariant data = widgetData(widget, m_objectWrapper->dataType(i));
        Q_ASSERT(data.isValid());
        m_objectWrapper->setData(i, data);
    }
}

QVariant GrpcForm::widgetData(QWidget *widget, const DataInfo::Type & type)
{
    if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
        return lineEdit->text();
    }
    if(QComboBox * comboEdit = qobject_cast<QComboBox*>(widget)) {
        if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(comboEdit->model())) {
            return model->data(model->index(comboEdit->currentIndex(), 0));
        } else {
            return comboEdit->itemData(comboEdit->currentIndex(), Qt::DisplayRole);
        }
    }
    if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
        return checkBox->isChecked();
    }
    if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
        QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
        QDateTime dateTime;
        if(dateEdit) {
            dateTime = dateEdit->dateTime();
        } else {
            dateTime = dateTimeEdit->dateTime();
        }
        return dateTime.toMSecsSinceEpoch();
    }
    if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
        QDateTime dateTime(QDate::currentDate(), timeEdit->time());
        return dateTime.toMSecsSinceEpoch();
    }
    if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
        return textEdit->toPlainText();
    }
    return QVariant();
}

void GrpcForm::hideAllButThis()
{
    if (QTabWidget * widget = tabWidget()) {
        int currentIndex = widget->indexOf(parentWidget());
        if(currentIndex > -1) {
            for(int it = 0; it < widget->count(); ++it) {
                if(it != currentIndex) {
                    widget->setTabVisible(it, false);
                } else {
                    widget->setTabVisible(it, true);
                }
            }
        }
    }
}

void GrpcForm::startInsert()
{
    clear();
    tabBar()->setTabIcon(tabIndex(), m_saveIcon);
}

void GrpcForm::startEdit()
{
    tabBar()->setTabIcon(tabIndex(), m_saveIcon);
}

void GrpcForm::finishSave()
{
    tabBar()->setTabIcon(tabIndex(), QIcon());
}

void GrpcForm::contentChanged()
{
    if(m_formFillingFinished) {
        emit formContentChanaged();
    }
}

QTabBar * GrpcForm::tabBar()
{
    if(QTabWidget * widget = tabWidget()) {
        return widget->tabBar();
    }
    return nullptr;
}

QTabWidget * GrpcForm::tabWidget()
{
    QWidget * currentParent = parentWidget();
    while (currentParent) {
        if (QTabWidget * tabWidget = qobject_cast<QTabWidget *>(currentParent)) {
            return tabWidget;
        }
        currentParent = currentParent->parentWidget();
    }
    return nullptr;
}

int GrpcForm::tabIndex()
{
    if (QTabWidget * widget = tabWidget()) {
        return  widget->indexOf(parentWidget());
    }
    return -1;
}

void GrpcForm::initilizeWidgets()
{
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        Q_ASSERT(widget);
        m_formWidgets << widget;
        widget->installEventFilter(this);
        // Connect content changes to GrpcTemplateController State
        if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            connect(lineEdit, &QLineEdit::textEdited, this, &GrpcForm::contentChanged);
        } else if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &GrpcForm::contentChanged);
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            connect(checkBox, &QCheckBox::checkStateChanged, this, &GrpcForm::contentChanged);
        } else if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
            QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(dateEdit) {
                connect(dateEdit, &QDateEdit::dateChanged, this, &GrpcForm::contentChanged);
            } else {
                connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &GrpcForm::contentChanged);
            }
        } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
            connect(timeEdit, &QTimeEdit::timeChanged, this, &GrpcForm::contentChanged);
        } else if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
            connect(textEdit, &QTextEdit::textChanged, this, &GrpcForm::contentChanged);
        } else {
            // It's not known managed widget yet
        }
    }
}

void GrpcForm::clear()
{
    m_formFillingFinished = false;
    // Clear all widgets
    m_formFillingFinished = true;
}

void GrpcForm::fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data)
{
    Q_ASSERT(data.isValid());
    if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::Double ||
                 type == DataInfo::String ||
                 type == DataInfo::Int ||
                 type == DataInfo::Int64);
        lineEdit->setText(data.toString());
    }
    if(QComboBox * comboEdit = qobject_cast<QComboBox*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::String ||
                 type == DataInfo::Int ||
                 type == DataInfo::Int64);
        if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(comboEdit->model())) {
            const QModelIndexList foundIndexList = model->match(model->index(0, 0), Qt::DisplayRole, data);
            if(foundIndexList.count() > 0) {
                comboEdit->setCurrentIndex(foundIndexList.at(0).row());
            }
        }
    }
    if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::Bool && data.typeId() == QMetaType::Bool);
        checkBox->setChecked(data.toBool());
    }
    if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
        QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::DateTime || type == DataInfo::DateTimeNoSec || type == DataInfo::Date);
        // Use by need DateTime or Date only
        // if DateTimeNoSec change format
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(data.toLongLong());
        if(dateEdit) {
            dateEdit->setDateTime(dateTime);
        } else {
            dateTimeEdit->setDateTime(dateTime);
        }
    }
    if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::Time);
    }
    if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::String);
        textEdit->setText(data.toString());
    }
}


