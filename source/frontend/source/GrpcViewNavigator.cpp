#include "GrpcViewNavigator.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace ScrollButtonHelper {
const char * leftHiddenButtonText = "...";
const char * rightHiddenButtonText = "...";
const char * prevButtonText = "<<";
const char * nextButtonText = ">>";
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
    Q_ASSERT(ScrollButtonHelper::maxPages > 0);
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

    if(m_currentPage > 1) {
        addButton(ScrollButtonHelper::PrevButton, -1, ScrollButtonHelper::prevButtonText, 0);
    }

    ScrollableButton * button;
    int i = from;
    for (; i <= to; ++i) {
        button = addButton(ScrollButtonHelper::NumberButton, i, QString::number(i));
        if(i == m_currentPage) {
            button->setChecked();
        }
    }

    if(last > to) {
        addButton(ScrollButtonHelper::RightHiddenButton, i, ScrollButtonHelper::rightHiddenButtonText);
        button = addButton(ScrollButtonHelper::LastButton, last, QString::number(last));
        if(last == m_currentPage) {
            button->setChecked();
        }
    }
    if(m_currentPage < last) {
        addButton(ScrollButtonHelper::NextButton, -1, ScrollButtonHelper::nextButtonText);
    }

    m_containerWidget->setUpdatesEnabled(true);
    m_containerWidget->updateGeometry();  // Recalculate layout once
}

void GrpcViewNavigator::addPages(int count)
{
    Q_ASSERT(count > 0);

    m_pages = count;
    clearAllButtons();
    m_currentPage = 1;
    addPages(1, std::min(ScrollButtonHelper::maxPages, count), count);
}

void GrpcViewNavigator::leftHidden()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        clearAllButtons();
        int from = button->queueNumber() - ScrollButtonHelper::maxPages + 1;
        if(from <= 0) {
            from = 1;
        }
        int to = from + ScrollButtonHelper::maxPages -1 ;
        if(from > 1) {
            ScrollableButton * firstButton = addButton(ScrollButtonHelper::FirstButton, 1, "1");
            if(m_currentPage == 1) {
                firstButton->setChecked();
            }
            addButton(ScrollButtonHelper::LeftHiddenButton, from - 1, ScrollButtonHelper::leftHiddenButtonText);
        }
        addPages(from, to, m_pages);
    }
}

void GrpcViewNavigator::rightHidden()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        clearAllButtons();

        ScrollableButton * firstButton = addButton(ScrollButtonHelper::FirstButton, 1, "1");
        if(m_currentPage > 1) {
            addButton(ScrollButtonHelper::PrevButton, -1, ScrollButtonHelper::prevButtonText);
        }
        if(m_currentPage == 1) {
            firstButton->setChecked();
        }
        addButton(ScrollButtonHelper::LeftHiddenButton, button->queueNumber() - 1, ScrollButtonHelper::leftHiddenButtonText);
        addPages(button->queueNumber(), std::min(button->queueNumber() + ScrollButtonHelper::maxPages - 1, m_pages), m_pages);
    }
}

void GrpcViewNavigator::prev()
{
    emit pageSelected(m_currentPage - 1);
    selectPage(m_currentPage - 1);
}

void GrpcViewNavigator::next()
{
    emit pageSelected(m_currentPage + 1);
    selectPage(m_currentPage + 1);
}

void GrpcViewNavigator::first()
{
    clearAllButtons();
    addPages(1, std::min(ScrollButtonHelper::maxPages, m_pages), m_pages);
}

void GrpcViewNavigator::last()
{
    clearAllButtons();

    if(m_pages > ScrollButtonHelper::maxPages) {
        addButton(ScrollButtonHelper::FirstButton, 1, "1");
        addButton(ScrollButtonHelper::LeftHiddenButton, m_pages - ScrollButtonHelper::maxPages, ScrollButtonHelper::leftHiddenButtonText);
        addPages(m_pages - ScrollButtonHelper::maxPages + 1, m_pages, m_pages);
    } else {
        addPages(1, std::min(ScrollButtonHelper::maxPages, m_pages), m_pages);
    }
}

ScrollableButton * GrpcViewNavigator::addButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, int position)
{
    auto btn = new ScrollableButton(type, numInQueue, text, m_centerWidget);
    if(position < 0) {
        m_buttonLayout->addWidget(btn);
    } else {
        m_buttonLayout->insertWidget(position, btn);
    }
    m_buttons.append(btn);

    if(type == ScrollButtonHelper::NumberButton) {
        connect(btn, &ScrollableButton::setCurrent, this, &GrpcViewNavigator::setPageCurrent);
        connect(btn, &ScrollableButton::buttonChecked, this, &GrpcViewNavigator::pageSelected);
    } else if(type == ScrollButtonHelper::LeftHiddenButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::leftHidden);
    } else if(type == ScrollButtonHelper::RightHiddenButton) {
        connect(btn, &QPushButton::clicked, this, &GrpcViewNavigator::rightHidden);
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
    m_pages = 0;
    m_currentPage = -1;
}

void GrpcViewNavigator::synchronizeByPages(int pageCount)
{
    if(pageCount <= 1) {
        m_currentPage = 1;
        addPages(pageCount);
        return;
    }

    if(m_pages > pageCount) {
        m_pages = pageCount;
        if(m_currentPage >= pageCount) {
            // The current page no longer exists
            // Make last-page current and select
            while(m_currentPage != pageCount) {
                m_currentPage --;
            }
            last();
        } else {
            selectPage(m_currentPage);
        }
    } else {
        m_pages = pageCount;
        selectPage(m_currentPage);
    }
}

void GrpcViewNavigator::synchronizeByRecords(int rowCount)
{
    int pageCount = static_cast<int>(std::ceil(static_cast<double>(rowCount) / ScrollButtonHelper::maxPages));
    synchronizeByPages(pageCount);
}

void GrpcViewNavigator::selectPage(int page)
{
    Q_ASSERT(m_pages >= page);
    // Recalculate, make visible possible buttons then select page
    int groupCount = static_cast<int>(std::ceil(static_cast<double>(m_pages) / ScrollButtonHelper::maxPages));
    int group = 0;
    int from = 1;
    int to = 1;
    for(; group < groupCount; ++group) {
        from = ScrollButtonHelper::maxPages * group + 1;
        to = from + ScrollButtonHelper::maxPages - 1;
        if(page >= from && page <= to) {
            group++;
            break;
        }
    }
    m_currentPage = page;
    if(groupCount == group) {
        // Last page
        last();
    } else if(group < groupCount){
        // Requires buttons First ad leftHidden
        clearAllButtons();
        if(group > 1) {
            addButton(ScrollButtonHelper::FirstButton, 1, "1");
            addButton(ScrollButtonHelper::LeftHiddenButton, from - 1, ScrollButtonHelper::leftHiddenButtonText);
        }
        addPages(from, to, m_pages);
    }
}

ScrollableButton::ScrollableButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, QWidget * parent)
    : QPushButton(text, parent)
    , m_numInQueue(numInQueue)
    , m_type(type)
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
