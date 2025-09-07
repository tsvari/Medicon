#ifndef GRPCTABLEVIEW_H
#define GRPCTABLEVIEW_H

#include <QTableView>

class GrpcTableView : public QTableView
{
    Q_OBJECT
public:
    GrpcTableView(QWidget * parent = nullptr);

signals:
    void focusIn();
    void focusOut();

protected:
    void focusInEvent(QFocusEvent * event) override;
    void focusOutEvent(QFocusEvent * event) override;
};

#endif // GRPCTABLEVIEW_H
