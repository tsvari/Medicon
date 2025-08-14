#ifndef GRPCFORM_H
#define GRPCFORM_H

#include <QWidget>

#include "GrpcObjectWrapper.hpp"

class GrpcUiTemplate;
class GrpcForm : public QWidget
{
    Q_OBJECT
public:
    explicit GrpcForm(QWidget *parent = nullptr);

    // purely virtual, be sure to override it in the child
    virtual void initializeData() = 0;

public slots:
    virtual void initializeWrapper(const QVariant & varData) = 0;

protected:
    friend class GrpcUiTemplate;
    IBaseGrpcObjectWrapper * m_objectWrapper = nullptr;
};

#endif // GRPCFORM_H
