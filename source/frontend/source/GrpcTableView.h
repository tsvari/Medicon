#ifndef GRPCTABLEVIEW_H
#define GRPCTABLEVIEW_H

#include <QTableView>

class QAction;

class GrpcTableView : public QTableView
{
    Q_OBJECT
public:
    explicit GrpcTableView(QWidget * parent = nullptr);
    ~GrpcTableView() override;

signals:
    void focusIn();
    void focusOut();
    void resizeToAdjustLoader();
    void escapePressed();

public slots:
    void select(int row);
    void clearRowSelection();
    void showWarning(const QString & warningTitle, const QString & message);

protected:
    void focusInEvent(QFocusEvent * event) override;
    void focusOutEvent(QFocusEvent * event) override;

    QAction * m_actionEscape = nullptr;
};

#endif // GRPCTABLEVIEW_H
