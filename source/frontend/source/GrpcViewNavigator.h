#ifndef GRPCVIEWNAVIGATOR_H
#define GRPCVIEWNAVIGATOR_H

#include <QPushButton>
#include <QVector>

namespace ScrollButtonHelper {
extern const char * prevButtonText;
extern const char * nextButtonText;
extern const int maxPages;
extern const char * styleSheet;

enum Type{NumberButton, PrevButton, NextButton, LastButton, FirstButton,};
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

public slots:
    void clearPages();
    void addPages(int count);
    void selectPage(int page);
    // The number of pages may change during navigation; this may also affect the current page
    void synchronize(int count);

private slots:
    void prev();
    void next();
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
