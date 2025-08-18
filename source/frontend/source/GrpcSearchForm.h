#ifndef GRPCSEARCHFORM_H
#define GRPCSEARCHFORM_H

#include <QObject>
#include <QWidget>

#include "JsonParameterFormatter.h"

class GrpcSearchForm : public QWidget
{
    Q_OBJECT

public:
    explicit GrpcSearchForm(QWidget *parent = nullptr);

public slots:
    virtual void submit();

signals:
    void startSearch(const JsonParameterFormatter & searchCriterias);
};

#endif // GRPCSEARCHFORM_H
