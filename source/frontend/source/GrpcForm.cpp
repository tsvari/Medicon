#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"
#include "GrpcTemplateController.h"

#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QEvent>
#include <QFocusEvent>

#include <QDebug>

GrpcForm::GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, IBaseGrpcObjectWrapper * masterObjectWrapper, QWidget *parent)
    : QWidget{parent}
    , m_objectWrapper(objectWrapper)
    , m_masterObjectWrapper(masterObjectWrapper)
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
    makeReadonly(false);
    m_formFillingFinished = false;
    const QVariant varData = index.data(GlobalRoles::VariantObjectRole);
    if(varData.isValid()) {
        m_objectWrapper->setObject(varData);
        Q_ASSERT(m_formWidgets.count() == m_objectWrapper->propertyCount());
        for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
            if(m_objectWrapper->dataMask(i) != DataMask::NoMask) {
                continue;
            }
            DataInfo::Type type = m_objectWrapper->dataType(i);
            fillWidget(m_formWidgets[i], type, m_objectWrapper->data(i));
        }
    }
    m_formFillingFinished = true;
}

void GrpcForm::fillObject()
{
    if(m_objectWrapper->hasObject()) {
        for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
            QVariant data ;
            QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
            Q_ASSERT(widget);
            if(m_objectWrapper->dataMask(i) == DataMask::ComboEditMask) {
                QComboBox * comboEdit = qobject_cast<QComboBox*>(widget);
                Q_ASSERT(comboEdit);
                data = comboEdit->currentText();
            } else if(m_objectWrapper->dataMask(i) == DataMask::CheckBoxMask) {
                QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget);
                Q_ASSERT(checkBox);
                if(checkBox->isChecked()) {
                    data = m_objectWrapper->trueData(i);
                } else {
                    data = m_objectWrapper->falseData(i);
                }
            } else {
                data = widgetData(widget, m_objectWrapper->dataType(i));
            }
            //Q_ASSERT(data.isValid());
            m_objectWrapper->setData(i, data);
        }
    }
}

void GrpcForm::masterChanged(const QModelIndex &index)
{
    // Will only be called in the slave template form
    m_masterObjectWrapper->setObject(index.data(GlobalRoles::VariantObjectRole));
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
        return dateTime.toSecsSinceEpoch();
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

void GrpcForm::prepareObject()
{
    fillObject();
    emit sendObject(object());
}

void GrpcForm::startInsert()
{
    clear();
    tabBar()->setTabIcon(tabIndex(), m_saveIcon);
    if(!m_formWidgets.empty()) {
        m_formWidgets.at(0)->setFocus();
    }
}

void GrpcForm::startEdit()
{
    tabBar()->setTabIcon(tabIndex(), m_saveIcon);
    m_formFillingFinished = true;
}

void GrpcForm::finishSave()
{
    tabBar()->setTabIcon(tabIndex(), QIcon());
}

bool GrpcForm::eventFilter(QObject *watched, QEvent *event)
{
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
        emit formEscapeSignal();
        return true;
    }
    // Pass the event to the next event filter or the watched object
   return QWidget::eventFilter(watched, event);

}

QVariant GrpcForm::masterVariantObject()
{
    if(m_masterObjectWrapper) {
        return m_masterObjectWrapper->variantObject();
    }
    return QVariant();
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
    QVariant newObject =  defaultObject();
    m_objectWrapper->setObject(newObject);

    m_formFillingFinished = false;
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        Q_ASSERT(widget);
        // Connect content changes to GrpcTemplateController State
        if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setText("");
        } else if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setCurrentIndex(-1);
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(false);
        } else if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
            QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(dateEdit) {
                dateEdit->setDate(QDate::currentDate());
            } else {
                dateTimeEdit->setDateTime(QDateTime::currentDateTime());
            }
        } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
            timeEdit->setTime(QTime(0,0,0,0));
        } else if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
            textEdit->setText("");
        } else {
            // It's not known managed widget yet
        }
    }
    m_formFillingFinished = true;
}

void GrpcForm::makeReadonly(bool readOnly)
{
    if(m_readonly == readOnly) {
        return;
    }
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        Q_ASSERT(widget);
        // Connect content changes to GrpcTemplateController State
        if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setReadOnly(readOnly);
        } else if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setEnabled(!readOnly);
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setEnabled(!readOnly);
        } else if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
                   QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(dateEdit) {
                dateEdit->setReadOnly(readOnly);
            } else {
                dateTimeEdit->setReadOnly(readOnly);
            }
        } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
            timeEdit->setReadOnly(readOnly);
        } else if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
            textEdit->setReadOnly(readOnly);
        } else {
            // It's not known managed widget yet
        }
    }
    m_readonly = readOnly;
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
        Q_ASSERT((type == DataInfo::Bool && data.typeId() == QMetaType::Bool) ||
                 type == DataInfo::String );
        checkBox->setChecked(data.toBool());
    }
    if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::Time);
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(data.toLongLong());
        timeEdit->setTime(dateTime.time());
    } else if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
        QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::DateTime || type == DataInfo::DateTimeNoSec || type == DataInfo::Date);
        // Use by need DateTime or Date only
        // if DateTimeNoSec change format
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(data.toLongLong());
        if(dateEdit) {
            dateEdit->setDateTime(dateTime);
        } else {
            dateTimeEdit->setDateTime(dateTime);
        }
        //https://forum.qt.io/topic/90945/time-zone-other-than-utc-and-local-in-qdatetimeedit/7
    }
    if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::String);
        textEdit->setText(data.toString());
    }
}


