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
    void resizeToAdjustLoader();

public slots:
    void select(int row);

protected:
    void focusInEvent(QFocusEvent * event) override;
    void focusOutEvent(QFocusEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
};

#endif // GRPCTABLEVIEW_H
