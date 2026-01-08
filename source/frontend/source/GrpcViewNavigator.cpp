#include "GrpcViewNavigator.h"

#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace {
int clampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

int windowStartForPage(int page, int pages, int windowSize)
{
    if (pages <= 0 || windowSize <= 0) return 1;
    // 1-based pages
    const int normalized = clampInt(page, 1, pages);
    return ((normalized - 1) / windowSize) * windowSize + 1;
}
}

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
    if(count <= 0) {
        clearPages();
        return;
    }

    m_pages = count;
    clearAllButtons();
    m_currentPage = 1;
    addPages(1, std::min(ScrollButtonHelper::maxPages, count), count);
}

void GrpcViewNavigator::leftHidden()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        const int from = button->queueNumber() - ScrollButtonHelper::maxPages + 1;
        rebuildWindow(from);
    }
}

void GrpcViewNavigator::rightHidden()
{
    if(ScrollableButton * button = qobject_cast<ScrollableButton*>(sender())) {
        rebuildWindow(button->queueNumber());
    }
}

void GrpcViewNavigator::prev()
{
    if(m_pages <= 0 || m_currentPage <= 1) {
        return;
    }
    emit pageSelected(m_currentPage - 1);
    selectPage(m_currentPage - 1);
}

void GrpcViewNavigator::next()
{
    if(m_pages <= 0 || m_currentPage >= m_pages) {
        return;
    }
    emit pageSelected(m_currentPage + 1);
    selectPage(m_currentPage + 1);
}

void GrpcViewNavigator::first()
{
    if(m_pages <= 0) {
        return;
    }
    selectPage(1);
}

void GrpcViewNavigator::last()
{
    if(m_pages <= 0) {
        return;
    }
    selectPage(m_pages);
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

void GrpcViewNavigator::rebuildWindow(int from)
{
    if(m_pages <= 0) {
        clearPages();
        return;
    }

    const int windowSize = ScrollButtonHelper::maxPages;
    const int maxFrom = (m_pages > windowSize) ? (m_pages - windowSize + 1) : 1;
    const int clampedFrom = clampInt(from, 1, maxFrom);
    const int to = std::min(clampedFrom + windowSize - 1, m_pages);

    clearAllButtons();

    if(clampedFrom > 1) {
        addButton(ScrollButtonHelper::FirstButton, 1, QString::number(1));
        addButton(ScrollButtonHelper::LeftHiddenButton, clampedFrom - 1, ScrollButtonHelper::leftHiddenButtonText);
    }

    addPages(clampedFrom, to, m_pages);
}

void GrpcViewNavigator::clearPages()
{
    clearAllButtons();
    m_pages = 0;
    m_currentPage = -1;
}

void GrpcViewNavigator::synchronizeByPages(int pageCount)
{
    if(pageCount <= 0) {
        clearPages();
        return;
    }

    if(pageCount == 1) {
        addPages(1);
        return;
    }

    if(m_currentPage < 1) {
        m_currentPage = 1;
    }

    if(m_pages > pageCount) {
        m_pages = pageCount;
        if(m_currentPage > pageCount) {
            // The current page no longer exists
            // Make last-page current and select
            m_currentPage = pageCount;
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
    if(m_pages <= 0) {
        return;
    }
    if(page < 1 || page > m_pages) {
        qCritical() << "GrpcViewNavigator::selectPage - Page out of range:" << page << "pages=" << m_pages;
        Q_ASSERT(page >= 1 && page <= m_pages);
        return;
    }
    m_currentPage = page;
    const int from = windowStartForPage(page, m_pages, ScrollButtonHelper::maxPages);
    rebuildWindow(from);
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
