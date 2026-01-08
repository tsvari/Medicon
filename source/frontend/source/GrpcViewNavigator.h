#ifndef GRPCVIEWNAVIGATOR_H
#define GRPCVIEWNAVIGATOR_H

#include <QPushButton>
#include <QVector>

/**
 * @file GrpcViewNavigator.h
 * @brief Pagination/navigation widget for GRPC template views.
 *
 * The navigator renders a compact, scrollable set of page buttons:
 *
 * - Page number buttons (checkable) that emit the selected page.
 * - Optional "first/last" buttons.
 * - Optional "prev/next" buttons.
 * - Optional "..." buttons to jump between windows of pages when there are
 *   more pages than can be displayed at once.
 *
 * Pages are 1-based. When there are no pages, @ref GrpcViewNavigator::currentPage
 * is -1.
 */

namespace ScrollButtonHelper {
/**
 * @brief Text for the left-side hidden-pages button ("...").
 */
extern const char * leftHiddenButtonText;
/**
 * @brief Text for the right-side hidden-pages button ("...").
 */
extern const char * rightHiddenButtonText;
/**
 * @brief Text for the "previous page" button.
 */
extern const char *  prevButtonText;
/**
 * @brief Text for the "next page" button.
 */
extern const char *  nextButtonText;
/**
 * @brief Maximum number of page-number buttons shown in a window.
 *
 * Note: This value is also used as the "records per page" constant when
 * converting record counts to page counts in
 * @ref GrpcViewNavigator::synchronizeByRecords.
 */
extern const int maxPages;
/**
 * @brief Common stylesheet applied to all navigator buttons.
 */
extern const char * styleSheet;

/**
 * @brief Button types used inside the navigator.
 */
enum Type{NumberButton, LeftHiddenButton, RightHiddenButton, LastButton, FirstButton, PrevButton, NextButton};
}

class GrpcViewNavigator;

/**
 * @brief Internal helper button used by @ref GrpcViewNavigator.
 *
 * Page number buttons are checkable and auto-exclusive, and emit:
 * - @ref setCurrent when they become checked (to update navigator state)
 * - @ref buttonChecked to notify listeners about a page selection
 */
class ScrollableButton : public QPushButton
{
    Q_OBJECT

private:
    friend class GrpcViewNavigator;
    explicit  ScrollableButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, QWidget * parent = nullptr);
    /**
     * @brief The page/window number associated with this button.
     *
     * For numbered buttons this is the 1-based page number.
     * For hidden/window navigation buttons this is the "anchor" page used to
     * rebuild the visible window.
     */
    int queueNumber() const {return m_numInQueue;}
    void setChecked();

signals:
    /**
     * @brief Emitted when a checkable page button becomes checked.
     * @param page 1-based page index.
     */
    void buttonChecked(int page);
    /**
     * @brief Emitted when a checkable page button becomes checked.
     * @param page 1-based page index.
     */
    void setCurrent(int page);

private:
    int m_numInQueue;
    ScrollButtonHelper::Type m_type;
};

class QScrollArea;
class QHBoxLayout;
class QPushButton;

/**
 * @brief A scrollable pagination widget used by GRPC template controllers.
 *
 * Typical usage:
 * - A controller connects a record count signal to @ref synchronizeByRecords.
 * - The controller connects @ref pageSelected to trigger loading the selected page.
 *
 * This widget is UI-thread only (Qt widgets are not thread-safe).
 */
class GrpcViewNavigator : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the navigator.
     * @param parent Parent widget.
     */
    explicit GrpcViewNavigator(QWidget * parent = nullptr);

    /**
     * @brief Returns the current 1-based page.
     *
     * Returns -1 when there are no pages.
     */
    int currentPage() const {return m_currentPage;}

    /**
     * @brief Returns the maximum number of page buttons displayed at once.
     */
    int maxPages() const {return ScrollButtonHelper::maxPages;}

public slots:
    /**
     * @brief Clears all pages and removes all buttons.
     *
     * After calling this, @ref currentPage returns -1.
     */
    void clearPages();

    /**
     * @brief Sets total number of pages and rebuilds the UI.
     *
     * If @p count <= 0, the navigator is cleared.
     * The current page becomes 1 when pages exist.
     */
    void addPages(int count);

    /**
     * @brief Selects a specific page (1-based) and rebuilds the visible window.
     *
     * This does not emit @ref pageSelected by itself.
     * Out-of-range pages are treated as a programming error (asserts in Debug).
     */
    void selectPage(int page);

    // The number of pages may change during navigation; this may also affect the current page
    /**
     * @brief Synchronizes the navigator using a page count.
     *
     * This may clamp the current page if the new page count is smaller.
     * If @p pageCount <= 0, the navigator is cleared.
     */
    void synchronizeByPages(int pageCount);

    /**
     * @brief Synchronizes the navigator using a record count.
     *
     * Page count is computed as: $\lceil \frac{\text{rowCount}}{\text{maxPages}} \rceil$.
     * If @p rowCount <= 0, the navigator is cleared.
     */
    void synchronizeByRecords(int rowCount);

private slots:
    /** @brief Handles clicking the left "..." (hidden pages) button. */
    void leftHidden();
    /** @brief Handles clicking the right "..." (hidden pages) button. */
    void rightHidden();
    /** @brief Selects the previous page if possible. */
    void prev();
    /** @brief Selects the next page if possible. */
    void next();
    /** @brief Selects page 1. */
    void first();
    /** @brief Selects the last page. */
    void last();
    /** @brief Updates internal current page without emitting @ref pageSelected. */
    void setPageCurrent(int page) {m_currentPage = page;}

signals:
    /**
     * @brief Emitted when the user selects a page via the UI.
     * @param page 1-based page index.
     */
    void pageSelected(int page);

private:
    ScrollableButton * addButton(ScrollButtonHelper::Type type, int numInQueue, const QString & text, int position = -1);
    void addPages(int from, int to, int last);
    /**
     * @brief Rebuilds the UI for a page window starting at @p from.
     *
     * @p from is clamped to a valid range based on the current page count.
     */
    void rebuildWindow(int from);
    void clearAllButtons();

private:
    QScrollArea * m_scrollArea;
    QWidget * m_containerWidget;
    QWidget * m_centerWidget;
    QHBoxLayout * m_buttonLayout;
    QVector<ScrollableButton *> m_buttons;

    int m_pages = 0;
    int m_currentPage = -1;
};


#endif // GRPCVIEWNAVIGATOR_H
