#include "GrpcForm.h"
#include "GrpcObjectTableModel.h"

#include <QApplication>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include <QEvent>
#include <QFocusEvent>

#include "GrpcImagePickerWidget.h"

#include <QDebug>

namespace {
    // ComboBox invalid index
    constexpr int INVALID_COMBOBOX_INDEX = -1;
}

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
        
        // Validate property count matches widget count
        if(m_formWidgets.count() != m_objectWrapper->propertyCount()) {
            qCritical() << "GrpcForm::fill - Property count mismatch: widgets =" 
                       << m_formWidgets.count() << ", properties =" 
                       << m_objectWrapper->propertyCount();
            Q_ASSERT(m_formWidgets.count() == m_objectWrapper->propertyCount());
            return;
        }
        
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
    if(!m_objectWrapper->hasObject()) {
        return;
    }
    
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QVariant data;
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        
        if(!widget) {
            qCritical() << "GrpcForm::fillObject - Widget not found:" 
                       << m_objectWrapper->propertyWidgetName(i).toString();
            Q_ASSERT(widget);
            continue;
        }

        // QLabel is a display-only widget in our forms.
        // Keep the existing wrapper value (e.g., image data) instead of trying to re-read it.
        if (qobject_cast<QLabel *>(widget)) {
            continue;
        }
        
        if(m_objectWrapper->dataMask(i) == DataMask::ComboEditMask) {
            QComboBox * comboEdit = qobject_cast<QComboBox*>(widget);
            if(!comboEdit) {
                qCritical() << "GrpcForm::fillObject - Expected QComboBox for property" << i;
                Q_ASSERT(comboEdit);
                continue;
            }
            data = comboEdit->currentText();
        } else if(m_objectWrapper->dataMask(i) == DataMask::CheckBoxMask) {
            QCheckBox * checkBox = qobject_cast<QCheckBox*>(widget);
            if(!checkBox) {
                qCritical() << "GrpcForm::fillObject - Expected QCheckBox for property" << i;
                Q_ASSERT(checkBox);
                continue;
            }
            if(checkBox->isChecked()) {
                data = m_objectWrapper->trueData(i);
            } else {
                data = m_objectWrapper->falseData(i);
            }
        } else {
            data = widgetData(widget, m_objectWrapper->dataType(i));
        }
        m_objectWrapper->setData(i, data);
    }
}

void GrpcForm::masterChanged(const QModelIndex &index)
{
    // Will only be called in the slave template form
    if(!m_masterObjectWrapper) {
        return;
    }
    
    if(!index.isValid()) {
        // Master selection cleared (e.g., model reset). Clear master wrapper.
        m_masterObjectWrapper->setObject(QVariant());
        return;
    }
    
    m_masterObjectWrapper->setObject(index.data(GlobalRoles::VariantObjectRole));
}

QVariant GrpcForm::widgetData(QWidget *widget, const DataInfo::Type & type)
{
    if(!widget) {
        qCritical() << "GrpcForm::widgetData - Null widget provided";
        Q_ASSERT(widget);
        return QVariant();
    }
    
    if(QComboBox * comboEdit = qobject_cast<QComboBox*>(widget)) {
        if(GrpcObjectTableModel * model = qobject_cast<GrpcObjectTableModel*>(comboEdit->model())) {
            return model->data(model->index(comboEdit->currentIndex(), 0));
        } else {
            return comboEdit->itemData(comboEdit->currentIndex(), Qt::DisplayRole);
        }
    } else if(QLineEdit * lineEdit = qobject_cast<QLineEdit*>(widget)) {
        if(type == DataInfo::Double) {
            return FrontConverter::to_locale_double(lineEdit->text());
        } else {
            return lineEdit->text();
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
    } else if (GrpcImagePickerWidget * picker = qobject_cast<GrpcImagePickerWidget *>(widget)) {
        Q_ASSERT(type == DataInfo::String);
        return picker->dataUrl();
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
    if (auto * bar = tabBar()) {
        const int index = tabIndex();
        if (index >= 0) {
            bar->setTabIcon(index, m_saveIcon);
        }
    }
    if(!m_formWidgets.empty()) {
        m_formWidgets.at(0)->setFocus();
    }
}

void GrpcForm::startEdit()
{
    if (auto * bar = tabBar()) {
        const int index = tabIndex();
        if (index >= 0) {
            bar->setTabIcon(index, m_saveIcon);
        }
    }
    m_formFillingFinished = true;
}

void GrpcForm::finishSave()
{
    if (auto * bar = tabBar()) {
        const int index = tabIndex();
        if (index >= 0) {
            bar->setTabIcon(index, QIcon());
        }
    }
}

bool GrpcForm::eventFilter(QObject *watched, QEvent *event)
{
    if(event && event->type() == QEvent::KeyPress) {
        auto * keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            emit formEscapeSignal();
            return true;
        }
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
        
        if(!widget) {
            qCritical() << "GrpcForm::initilizeWidgets - Widget not found:" 
                       << m_objectWrapper->propertyWidgetName(i).toString();
            Q_ASSERT(widget);
            continue;
        }

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
        } else if (GrpcImagePickerWidget * picker = qobject_cast<GrpcImagePickerWidget *>(widget)) {
            connect(picker, &GrpcImagePickerWidget::dataUrlChanged, this, &GrpcForm::contentChanged);
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
        
        if(!widget) {
            qCritical() << "GrpcForm::clear - Widget not found:" 
                       << m_objectWrapper->propertyWidgetName(i).toString();
            Q_ASSERT(widget);
            continue;
        }
        
        // Connect content changes to GrpcTemplateController State
        if(QComboBox * comboBox = qobject_cast<QComboBox*>(widget)) {
            comboBox->setCurrentIndex(INVALID_COMBOBOX_INDEX);
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
        } else if (GrpcImagePickerWidget * picker = qobject_cast<GrpcImagePickerWidget *>(widget)) {
            // For inserts, keep default-object image visible (if any).
            picker->setDataUrl(m_objectWrapper->data(i).toString());
        } else if(QLabel * label = qobject_cast<QLabel*>(widget)) {
            label->setPixmap(QPixmap());
            label->setText("");
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
        
        if(!widget) {
            qCritical() << "GrpcForm::makeReadonly - Widget not found:" 
                       << m_objectWrapper->propertyWidgetName(i).toString();
            Q_ASSERT(widget);
            continue;
        }
        
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
        } else if (GrpcImagePickerWidget * picker = qobject_cast<GrpcImagePickerWidget *>(widget)) {
            picker->setReadOnly(readOnly);
        } else {
            // It's not known managed widget yet
        }
    }
    m_readonly = readOnly;
}

void GrpcForm::fillWidget(QWidget * widget, const DataInfo::Type & type, const QVariant & data)
{
    if(!widget) {
        qCritical() << "GrpcForm::fillWidget - Null widget provided";
        Q_ASSERT(widget);
        return;
    }
    
    if(!data.isValid()) {
        qCritical() << "GrpcForm::fillWidget - Invalid data provided";
        Q_ASSERT(data.isValid());
        return;
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
    } else if (GrpcImagePickerWidget * picker = qobject_cast<GrpcImagePickerWidget *>(widget)) {
        Q_ASSERT(type == DataInfo::String);
        picker->setDataUrl(data.toString());
    } else if(QLabel * label = qobject_cast<QLabel*>(widget)) {
        Q_ASSERT(type == DataInfo::String);

        const QString s = data.toString();
        if (s.isEmpty()) {
            label->setPixmap(QPixmap());
            label->setText("");
            return;
        }

        // Support data urls: data:image/png;base64,... and data:image/jpeg;base64,...
        static const QString dataPrefix = QStringLiteral("data:image/");
        static const QString base64Marker = QStringLiteral(";base64,");
        if (s.startsWith(dataPrefix)) {
            const int markerPos = s.indexOf(base64Marker);
            if (markerPos > 0) {
                const int payloadStart = markerPos + base64Marker.size();
                const QByteArray b64 = s.mid(payloadStart).toLatin1();
                const QByteArray bytes = QByteArray::fromBase64(b64);
                QPixmap pix;
                if (pix.loadFromData(bytes)) {
                    label->setPixmap(pix);
                    return;
                }
            }
        }

        // Fallback: treat as resource/file path.
        QPixmap pix(s);
        if (!pix.isNull()) {
            label->setPixmap(pix);
            return;
        }

        // Final fallback: show as text.
        label->setText(s);
    }
}


