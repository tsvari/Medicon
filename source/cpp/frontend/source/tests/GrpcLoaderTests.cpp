#include "GrpcThreadWorker.h"

#include <gtest/gtest.h>

#include <QApplication>
#include <QCoreApplication>
#include <QTest>
#include <QWidget>

namespace {

QString loaderGifPath()
{
    // Defined by cmake/global-settings.cmake via add_definitions.
    return QString::fromUtf8(ALL_FRONTEND_PROJECT_PATH) + "source/icons/loaderSmall.gif";
}

} // namespace

TEST(GrpcLoaderTest, CalculatePositionCenter)
{
    const QSize parentSize(100, 100);
    const QSize contentSize(20, 10);

    const QPoint pos = GrpcLoader::calculatePosition(parentSize, contentSize, GrpcLoader::Center, 4);

    EXPECT_EQ(pos, QPoint(40, 45));
}

TEST(GrpcLoaderTest, CalculatePositionTopCenter)
{
    const QSize parentSize(100, 100);
    const QSize contentSize(20, 10);

    const QPoint pos = GrpcLoader::calculatePosition(parentSize, contentSize, GrpcLoader::TopCenter, 4);

    EXPECT_EQ(pos, QPoint(40, 4));
}

TEST(GrpcLoaderTest, CalculatePositionTopRight)
{
    const QSize parentSize(100, 100);
    const QSize contentSize(20, 10);

    const QPoint pos = GrpcLoader::calculatePosition(parentSize, contentSize, GrpcLoader::TopRight, 4);

    EXPECT_EQ(pos, QPoint(76, 4));
}

TEST(GrpcLoaderTest, ShowLoaderStartsAndPositions)
{
    QWidget parent;
    parent.resize(240, 180);
    parent.show();

    GrpcLoader loader(loaderGifPath(), GrpcLoader::Center, &parent);

    loader.showLoader(true);
    QCoreApplication::processEvents();

    ASSERT_TRUE(loader.isVisible());

    const QPoint expected = GrpcLoader::calculatePosition(parent.rect().size(), loader.size(), GrpcLoader::Center, 4);
    EXPECT_EQ(loader.pos(), expected);

    loader.showLoader(false);
    QCoreApplication::processEvents();

    EXPECT_FALSE(loader.isVisible());
}

TEST(GrpcLoaderTest, ParentResizeAutoAdjustsWhenVisible)
{
    QWidget parent;
    parent.resize(200, 120);
    parent.show();

    GrpcLoader loader(loaderGifPath(), GrpcLoader::TopRight, &parent);
    loader.showLoader(true);
    QCoreApplication::processEvents();

    parent.resize(320, 240);
    QCoreApplication::processEvents();

    const QPoint expected = GrpcLoader::calculatePosition(parent.rect().size(), loader.size(), GrpcLoader::TopRight, 4);
    EXPECT_EQ(loader.pos(), expected);
}
