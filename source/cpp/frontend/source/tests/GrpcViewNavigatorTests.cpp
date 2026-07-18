#include "GrpcViewNavigator.h"

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QMetaObject>
#include <QSignalSpy>
#include <QTest>

namespace {

ScrollableButton * findPageButton(GrpcViewNavigator & navigator, const QString & text)
{
    const auto buttons = navigator.findChildren<ScrollableButton *>();
    for (auto * btn : buttons) {
        if (btn && btn->text() == text) {
            return btn;
        }
    }
    return nullptr;
}

ScrollableButton * findHiddenButton(GrpcViewNavigator & navigator)
{
    const auto buttons = navigator.findChildren<ScrollableButton *>();
    for (auto * btn : buttons) {
        if (!btn) {
            continue;
        }
        // Hidden buttons are rendered as "..." and are not checkable.
        if (btn->text() == "..." && !btn->isCheckable()) {
            return btn;
        }
    }
    return nullptr;
}

} // namespace

TEST(GrpcViewNavigatorTest, SynchronizeByRecordsZeroClearsPages)
{
    GrpcViewNavigator navigator;

    navigator.synchronizeByRecords(0);

    EXPECT_EQ(navigator.currentPage(), -1);
    EXPECT_TRUE(navigator.findChildren<ScrollableButton *>().isEmpty());
}

TEST(GrpcViewNavigatorTest, AddPagesCreatesButtonsAndClickEmitsPageSelected)
{
    GrpcViewNavigator navigator;
    navigator.addPages(3);

    QSignalSpy spy(&navigator, &GrpcViewNavigator::pageSelected);

    auto * page2 = findPageButton(navigator, "2");
    ASSERT_NE(page2, nullptr);

    QTest::mouseClick(page2, Qt::LeftButton);
    QCoreApplication::processEvents();

    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toInt(), 2);
    EXPECT_EQ(navigator.currentPage(), 2);
}

TEST(GrpcViewNavigatorTest, SynchronizeByPagesClampCurrentPageWhenReducing)
{
    GrpcViewNavigator navigator;
    navigator.addPages(10);
    navigator.selectPage(10);

    navigator.synchronizeByPages(3);

    EXPECT_EQ(navigator.currentPage(), 3);
}

TEST(GrpcViewNavigatorTest, SynchronizeByPagesDoesNotEmitPageSelected)
{
    GrpcViewNavigator navigator;
    navigator.addPages(10);
    navigator.selectPage(5);

    QSignalSpy spy(&navigator, &GrpcViewNavigator::pageSelected);
    navigator.synchronizeByPages(3);
    QCoreApplication::processEvents();

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(navigator.currentPage(), 3);
}

TEST(GrpcViewNavigatorTest, ClickingHiddenButtonShiftsVisibleWindow)
{
    GrpcViewNavigator navigator;
    navigator.addPages(50);

    // Initially the first window is 1..maxPages, so "21" should not be present.
    ASSERT_EQ(findPageButton(navigator, "21"), nullptr);

    // There should be exactly one hidden button at the start (right hidden).
    auto * hidden = findHiddenButton(navigator);
    ASSERT_NE(hidden, nullptr);

    QTest::mouseClick(hidden, Qt::LeftButton);
    QCoreApplication::processEvents();

    // After shifting window, the next page window should become visible.
    EXPECT_NE(findPageButton(navigator, "21"), nullptr);
}

TEST(GrpcViewNavigatorTest, PrevNextDoNotEmitWhenOutOfRange)
{
    GrpcViewNavigator navigator;
    navigator.addPages(1);

    QSignalSpy spy(&navigator, &GrpcViewNavigator::pageSelected);

    ASSERT_TRUE(QMetaObject::invokeMethod(&navigator, "prev"));
    ASSERT_TRUE(QMetaObject::invokeMethod(&navigator, "next"));

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(navigator.currentPage(), 1);
}
