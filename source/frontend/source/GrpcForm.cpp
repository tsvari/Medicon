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
    if(QLineEdit * doubleLineEdit = qobject_cast<QLineEdit*>(widget); type == DataInfo::Double) {
        return FrontConverter::to_locale_double(doubleLineEdit->text());
    } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
        return lineEdit->text();
    } else if(QComboBox * comboEdit = qobject_cast<QComboBox*>(widget)) {
        if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(comboEdit->model())) {
            return model->data(model->index(comboEdit->currentIndex(), 0));
        } else {
            return comboEdit->itemData(comboEdit->currentIndex(), Qt::DisplayRole);
        }
    } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
        return checkBox->isChecked();
    } else if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(widget);
        QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
        QDateTime dateTime;
        if(dateEdit) {
            dateTime = dateEdit->dateTime();
        } else {
            dateTime = dateTimeEdit->dateTime();
        }
        return dateTime.toSecsSinceEpoch();
    } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
        QDateTime dateTime(QDate::currentDate(), timeEdit->time());
        return dateTime.toMSecsSinceEpoch();
    } else if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
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

        const DataInfo::Type & type = m_objectWrapper->dataType(i);
        QLocale systemLocale = QLocale::system();

        widget->installEventFilter(this);
        m_formWidgets << widget;

        if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &GrpcForm::contentChanged);
        } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            if(type == DataInfo::Double) {
                connect(lineEdit, &QLineEdit::textEdited, this, &GrpcForm::contentChanged);
                // Set validator
                auto * validator = new QDoubleValidator(this);
                validator->setLocale(QLocale::system());
                lineEdit->setValidator(validator);
                connect(lineEdit, &QLineEdit::editingFinished, lineEdit, [type, systemLocale, lineEdit]() {
                    bool ok;
                    double doublVal = systemLocale.toDouble(lineEdit->text(), &ok);
                    if (ok) {
                        QString strValue = FrontConverter::to_qvariant_by_type(QVariant::fromValue(doublVal), type).toString();
                        lineEdit->setText(strValue);
                    } else {
                        lineEdit->selectAll();
                        lineEdit->setFocus();
                    }
                });
            } else if(type == DataInfo::Int || type == DataInfo::Int64) {
                connect(lineEdit, &QLineEdit::textEdited, this, &GrpcForm::contentChanged);
                auto * validator = new QIntValidator(this);
                validator->setLocale(QLocale::system());
                lineEdit->setValidator(validator);
                connect(lineEdit, &QLineEdit::editingFinished, lineEdit, [systemLocale, lineEdit]() {
                    bool ok;
                    int intVal = systemLocale.toInt(lineEdit->text(), &ok);
                    if (ok) {
                        lineEdit->setText(QString::number(intVal));
                    } else {
                        lineEdit->selectAll();
                        lineEdit->setFocus();
                    }
                });
            } else if(type == DataInfo::String) {
                connect(lineEdit, &QLineEdit::textEdited, this, &GrpcForm::contentChanged);
                // Allow only letters, numbers, underscores, spaces, and dashes (Unicode aware)
                QRegularExpression safeBase("^[\\p{L}\\p{N}_\\s-]*$",
                                            QRegularExpression::UseUnicodePropertiesOption);
                lineEdit->setValidator(new QRegularExpressionValidator(safeBase, this));

                connect(lineEdit, &QLineEdit::textEdited, lineEdit, [lineEdit]() {
                    static const QRegularExpression forbiddenPatterns(
                        R"((--|;|'|"|/\*|\*/|\\|#))",
                        QRegularExpression::UseUnicodePropertiesOption);
                    QString clean, text = lineEdit->text();
                    clean = text;
                    clean.remove(forbiddenPatterns);

                    // If something was removed, update the text quietly
                    if (clean != text) {
                        int cursor = lineEdit->cursorPosition();
                        lineEdit->setText(clean);
                        //lineEdit->setCursorPosition(std::min(cursor - 1, clean.length()));
                    }
                });
            }
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            connect(checkBox, &QCheckBox::checkStateChanged, this, &GrpcForm::contentChanged);
        } else if(QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(dateTimeEdit)) {
                connect(dateEdit, &QDateEdit::dateChanged, this, &GrpcForm::contentChanged);
                dateEdit->setDisplayFormat(systemLocale.dateFormat(QLocale::ShortFormat));
            } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(dateTimeEdit)) {
                connect(timeEdit, &QTimeEdit::timeChanged, this, &GrpcForm::contentChanged);
                timeEdit->setDisplayFormat(systemLocale.timeFormat(QLocale::ShortFormat));
            } else {
                if(type == DataInfo::DateTime) {
                    connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &GrpcForm::contentChanged);
                    // DataInfo::DateTime requires secconds
                    // So add hardcoded seconds to the format
                    QString fmt = systemLocale.dateTimeFormat(QLocale::ShortFormat);
                    if (!fmt.contains("s")) {
                        fmt.replace("mm", "mm:ss");
                    }
                    dateTimeEdit->setDisplayFormat(fmt);
                } else if(type == DataInfo::DateTimeNoSec) {
                    connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &GrpcForm::contentChanged);
                    dateTimeEdit->setDisplayFormat(systemLocale.dateTimeFormat(QLocale::ShortFormat));
                }
            }
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
        if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setCurrentIndex(-1);
        } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setText("");
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setChecked(false);
        } else if(QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(dateTimeEdit)) {
                dateEdit->setDate(QDate::currentDate());
            } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(dateTimeEdit)) {
                timeEdit->setTime(QTime::currentTime());
            } else {
                dateTimeEdit->setDateTime(QDateTime::currentDateTime());
            }
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
        if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setEnabled(!readOnly);
        } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
            lineEdit->setReadOnly(readOnly);
        } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
            checkBox->setEnabled(!readOnly);
        } else if(QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
            if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(dateTimeEdit)) {
                dateEdit->setReadOnly(readOnly);
            } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(dateTimeEdit)) {
                timeEdit->setReadOnly(readOnly);
            } else {
                dateTimeEdit->setReadOnly(readOnly);
            }
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
    } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
        if(type == DataInfo::Double) {
            lineEdit->setText(FrontConverter::to_qvariant_by_type(data, type).toString());
        } else {
            // Check type should be suitable
            Q_ASSERT(type == DataInfo::String ||
                     type == DataInfo::Int ||
                     type == DataInfo::Int64);
            lineEdit->setText(data.toString());
        }
    } else if(QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget)) {
        // Check type should be suitable
        Q_ASSERT((type == DataInfo::Bool && data.typeId() == QMetaType::Bool) ||
                 type == DataInfo::String );
        checkBox->setChecked(data.toBool());
    } else if(QDateTimeEdit * dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget)) {
        QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(data.toLongLong());
        if(QDateEdit * dateEdit = qobject_cast<QDateEdit*>(dateTimeEdit)) {
            dateEdit->setDate(dateTime.date());
        } else if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(dateTimeEdit)) {
            timeEdit->setTime(dateTime.time());
        } else {
            dateTimeEdit->setDateTime(dateTime);
        }
    } else if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::String);
        textEdit->setText(data.toString());
    }
}


