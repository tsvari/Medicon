#ifndef GRPCVIEWNAVIGATOR_H
#define GRPCVIEWNAVIGATOR_H

#include <QPushButton>
#include <QVector>

namespace ScrollButtonHelper {
extern const char * leftHiddenButtonText;
extern const char * rightHiddenButtonText;
extern const int maxPages;
extern const char * styleSheet;

enum Type{NumberButton, LeftHiddenButton, RightHiddenButton, LastButton, FirstButton,};
}

class GrpcViewNavigator;
class ScrollableButton : public QPushButton
{
    Q_OBJECT

private:
    friend class GrpcViewNavigator;
    explicit  ScrollableButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, QWidget * parent = nullptr);
    int queueNumber() {return m_numInQueue;}
    void setChecked();

signals:
    void buttonChecked(int page);
    void setCurrent(int page);

private:
    int m_numInQueue;
};

class QScrollArea;
class QHBoxLayout;
class QPushButton;
class GrpcViewNavigator : public QWidget
{
    Q_OBJECT
public:
    explicit GrpcViewNavigator(QWidget * parent = nullptr);

    int currentPage() {return m_currentPage;}
    int maxPages() {return ScrollButtonHelper::maxPages;}

public slots:
    void clearPages();
    void addPages(int count);
    void selectPage(int page);
    // The number of pages may change during navigation; this may also affect the current page
    void synchronizeByPages(int pageCount);
    void synchronizeByRecords(int rowCount);

private slots:
    void leftHidden();
    void rightHidden();
    void first();
    void last();
    void setPageCurrent(int page) {m_currentPage = page;}

signals:
    void pageSelected(int page);

private:
    ScrollableButton * addButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text);
    void addPages(int from, int to, int last);
    void clearAllButtons();

private:
    QScrollArea * m_scrollArea;
    QWidget * m_containerWidget;
    QWidget * m_centerWidget;
    QHBoxLayout * m_buttonLayout;
    QVector<QPushButton *> m_buttons;

    int m_pages = 0;
    int m_currentPage = -1;
};


#endif // GRPCVIEWNAVIGATOR_H
