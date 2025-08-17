#include "GrpcForm.h"
#include "GrpcUiTemplate.h"
#include "GrpcObjectTableModel.h"

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QTextEdit>

GrpcForm::GrpcForm(IBaseGrpcObjectWrapper * objectWrapper, QWidget *parent)
    : QWidget{parent}
    , m_objectWrapper(objectWrapper)
{}

GrpcForm::~GrpcForm()
{
    delete m_objectWrapper;
}

QVariant GrpcForm::object()
{
    return m_objectWrapper->variantObject();
}

void GrpcForm::fillForm(const QModelIndex &index)
{
    const QVariant varData = index.data(GlobalRoles::VariantObjectRole);
    if(varData.isValid()) {
        m_objectWrapper->setObject(varData);
        Q_ASSERT(m_formWidgets.count() == m_objectWrapper->propertyCount());
        for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
            DataInfo::Type type = m_objectWrapper->dataType(i);
            fillWidget(m_formWidgets[i], type, m_objectWrapper->data(i));
        }
    }
}

void GrpcForm::initializeData()
{
    for(int i = 0; i < m_objectWrapper->propertyCount(); ++i) {
        QWidget * widget = findChild<QWidget*>(m_objectWrapper->propertyWidgetName(i).toString());
        Q_ASSERT(widget);
        m_formWidgets << widget;
    }
}

void GrpcForm::fillObject()
{

}

void GrpcForm::clearForm()
{

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
    }
    if(QTimeEdit * timeEdit = qobject_cast<QTimeEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::Time);
    }
    if(QTextEdit * textEdit = qobject_cast<QTextEdit*>(widget)) {
        // Check type should be suitable
        Q_ASSERT(type == DataInfo::String);
    }
}
