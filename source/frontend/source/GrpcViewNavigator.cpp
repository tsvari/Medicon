#include "GrpcViewNavigator.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace ScrollButtonHelper {
const char * prevButtonText = "...";
const char * nextButtonText = "...";
const int maxPages = 20;
const char * styleSheet ="QPushButton {"
                         "    border: 1px solid #aaa;"
                         "    border-radius: 2px;"
                         "    padding: 4px;"
                         "}"
                         "QPushButton:hover {"
                         "    background-color: #d0d0d0;"   /* hover color */
                         "}"
                         "QPushButton:checked {"
                         "    background-color: #0078D7;"   /* pressed color */
                         "    color: white;"
                         "    border: 1px solid #005a9e;"
                         "}"
                         "QPushButton:checked:hover {"
                         "    background-color: #3399ff;"   /* hover while pressed */
                         "}";
}

GrpcViewNavigator::GrpcViewNavigator(QWidget * parent)
    : QWidget(parent)
{
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setLineWidth(0);
    m_scrollArea->setMidLineWidth(0);

    // Container widget fills the scroll area
    m_containerWidget = new QWidget();

    // Center widget holds the actual buttons
    m_centerWidget = new QWidget(m_containerWidget);
    m_buttonLayout = new QHBoxLayout(m_centerWidget);
    m_buttonLayout->setSpacing(1);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);

    // --- Center horizontally & vertically ---
    auto vCenterLayout = new QVBoxLayout(m_containerWidget);
    vCenterLayout->setContentsMargins(0, 0, 0, 0);
    vCenterLayout->addStretch();                              // top stretch
    vCenterLayout->addWidget(m_centerWidget, 0, Qt::AlignHCenter); // center horizontally
    vCenterLayout->addStretch();                              // bottom stretch
    // ---------------------------------------

    m_scrollArea->setWidget(m_containerWidget);

    // Main layout for the panel itself
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);
}


void GrpcViewNavigator::addPages(int from, int to, int last)
{
    // Temporarily disable updates and layout recalculation
    m_containerWidget->setUpdatesEnabled(false);

    ScrollableButton * button;
    int i = from;
    for (; i <= to; ++i) {
        button = addButton(ScrollButtonHelper::NumberButton, i, QString::number(i));
        if(i == m_currentPage) {
            button->setChecked();
        }
    }

    if(last > to) {
        addButton(ScrollButtonHelper::NextButton, i, ScrollButtonHelper::nextButtonText);
        button = addButton(ScrollButtonHelper::LastButton, last, QString::number(last));
        if(last == m_currentPage) {
            button->setChecked();
        }
    }
    m_containerWidget->setUpdatesEnabled(true);
    m_containerWidget->updateGeometry();  // Recalculate layout once
}

void GrpcViewNavigator::addPages(int count)
{
    Q_ASSERT(count > 0);

    m_allPages = count;
    clearAllButtons();
    m_currentPage = 1;
    addPages(1, std::min(ScrollButtonHelper::maxPages, count), count);
}

void GrpcViewNavigator::prev()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        int num = button->queueNumber();
        clearAllButtons();
        int from = num - ScrollButtonHelper::maxPages + 1;
        if(from <= 0) {
            from = 1;
        }
        int to = from + ScrollButtonHelper::maxPages -1 ;
        if(from > 1) {
            ScrollableButton * firstButton = addButton(ScrollButtonHelper::FirstButton, 1, "1");
            if(m_currentPage == 1) {
                firstButton->setChecked();
            }
            addButton(ScrollButtonHelper::PrevButton, from - 1, ScrollButtonHelper::prevButtonText);
        }
        addPages(from, to, m_allPages);
    }
}

void GrpcViewNavigator::next()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        int num = button->queueNumber();
        clearAllButtons();

        ScrollableButton * firstButton = addButton(ScrollButtonHelper::FirstButton, 1, "1");
        if(m_currentPage == 1) {
            firstButton->setChecked();
        }
        addButton(ScrollButtonHelper::PrevButton, num - 1, ScrollButtonHelper::prevButtonText);
        addPages(num, std::min(num + ScrollButtonHelper::maxPages - 1, m_allPages), m_allPages);
    }
}

void GrpcViewNavigator::first()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        clearAllButtons();
        addPages(1, ScrollButtonHelper::maxPages, m_allPages);
    }
}

void GrpcViewNavigator::last()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        clearAllButtons();

        addButton(ScrollButtonHelper::FirstButton, 1, "1");
        addButton(ScrollButtonHelper::PrevButton, m_allPages - ScrollButtonHelper::maxPages - 1, ScrollButtonHelper::prevButtonText);

        addPages(m_allPages - ScrollButtonHelper::maxPages + 1, m_allPages, m_allPages);
    }
}

ScrollableButton *  GrpcViewNavigator::addButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text)
{
    auto btn = new ScrollableButton(type, numInQueue, text, m_centerWidget);
    m_buttonLayout->addWidget(btn);
    m_buttons.append(btn);

    if(type == ScrollButtonHelper::NumberButton) {
        connect(btn, &ScrollableButton::setCurrent, this, &GrpcViewNavigator::setPageCurrent);
        connect(btn, &ScrollableButton::buttonChecked, this, &GrpcViewNavigator::pageSelected);
    } else if(type == ScrollButtonHelper::PrevButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::prev);
    } else if(type == ScrollButtonHelper::NextButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::next);
    } else if(type == ScrollButtonHelper::FirstButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::first);
        connect(btn, &ScrollableButton::setCurrent, this, &GrpcViewNavigator::setPageCurrent);
        connect(btn, &ScrollableButton::buttonChecked, this, &GrpcViewNavigator::pageSelected);
    } else if(type == ScrollButtonHelper::LastButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::last);
        connect(btn, &ScrollableButton::setCurrent, this, &GrpcViewNavigator::setPageCurrent);
        connect(btn, &ScrollableButton::buttonChecked, this, &GrpcViewNavigator::pageSelected);
    }
    return btn;
}

void GrpcViewNavigator::clearAllButtons()
{
    // Disable updates temporarily for performance
    m_containerWidget->setUpdatesEnabled(false);

    for (auto btn : std::as_const(m_buttons)) {
        m_buttonLayout->removeWidget(btn);
        btn->deleteLater();  // safe deferred deletion
    }

    m_buttons.clear();

    m_containerWidget->setUpdatesEnabled(true);
    m_containerWidget->updateGeometry();
}

void GrpcViewNavigator::clearPages()
{
    clearAllButtons();
    m_allPages = 0;
    m_currentPage = -1;
}

ScrollableButton::ScrollableButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, QWidget * parent)
    : QPushButton(text, parent)
    , m_numInQueue(numInQueue)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    int height = fontMetrics().height() * 1.5;
    int width = fontMetrics().maxWidth() * 1.2;
    setFixedSize(width, height);

    if(type == ScrollButtonHelper::NumberButton ||
        type == ScrollButtonHelper::LastButton ||
        type == ScrollButtonHelper::FirstButton)
    {
        // Make button toggleable
        setCheckable(true);
        setAutoExclusive(true); //  ensures only one stays checked at a time

        connect(this, &QPushButton::toggled, this, [this, numInQueue](bool checked) {
            if (!checked) {
                return; // only emit when becomes checked
            }
            if (numInQueue >= 0) {
                emit setCurrent(numInQueue);
                emit buttonChecked(numInQueue);
            }
        });
    }
    setStyleSheet(ScrollButtonHelper::styleSheet);
}

void ScrollableButton::setChecked()
{
    blockSignals(true);
    QPushButton::setChecked(true);
    blockSignals(false);
}
