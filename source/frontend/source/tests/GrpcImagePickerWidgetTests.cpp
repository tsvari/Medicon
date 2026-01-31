/**
 * @file GrpcImagePickerWidgetTests.cpp
 * @brief Unit tests for GrpcImagePickerWidget
 */

#include <gtest/gtest.h>

#include "../GrpcImagePickerWidget.h"

#include <QBuffer>
#include <QDir>
#include <QImage>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>

using namespace testing;

TEST(GrpcImagePickerWidgetTests, Defaults)
{
    GrpcImagePickerWidget w;

    EXPECT_FALSE(w.readOnly());
    EXPECT_EQ(w.maxFileSizeBytes(), 100 * 1024);
    EXPECT_TRUE(w.showImageButtonVisible());
    EXPECT_TRUE(w.clearImageButtonVisible());
    EXPECT_TRUE(w.lastError().isEmpty());
}

TEST(GrpcImagePickerWidgetTests, ReadOnlyDisablesButtons)
{
    GrpcImagePickerWidget w;

    ASSERT_NE(w.showImageButton(), nullptr);
    ASSERT_NE(w.clearImageButton(), nullptr);

    w.setReadOnly(true);
    EXPECT_FALSE(w.showImageButton()->isEnabled());
    EXPECT_FALSE(w.clearImageButton()->isEnabled());

    w.setReadOnly(false);
    EXPECT_TRUE(w.showImageButton()->isEnabled());
    EXPECT_TRUE(w.clearImageButton()->isEnabled());
}

TEST(GrpcImagePickerWidgetTests, SetDataUrlEmitsSignal)
{
    GrpcImagePickerWidget w;
    QSignalSpy spy(&w, &GrpcImagePickerWidget::dataUrlChanged);

    QImage img(8, 8, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::red);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
    ASSERT_TRUE(img.save(&buffer, "PNG"));

    const QString dataUrl = QStringLiteral("data:image/png;base64,%1")
                                .arg(QString::fromLatin1(bytes.toBase64()));

    w.setDataUrl(dataUrl);
    EXPECT_EQ(w.dataUrl(), dataUrl);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(w.lastError().isEmpty());
}

TEST(GrpcImagePickerWidgetTests, LoadFromFileHonorsMaxSize)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    const QString filePath = QDir(tmpDir.path()).filePath("img.png");

    QImage img(8, 8, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::green);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    ASSERT_TRUE(buffer.open(QIODevice::WriteOnly));
    ASSERT_TRUE(img.save(&buffer, "PNG"));

    {
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        ASSERT_EQ(f.write(bytes), bytes.size());
        f.close();
    }

    GrpcImagePickerWidget w;
    w.setMaxFileSizeBytes(10); // too small

    QSignalSpy errSpy(&w, &GrpcImagePickerWidget::lastErrorChanged);
    EXPECT_FALSE(w.loadFromFile(filePath));
    EXPECT_FALSE(w.lastError().isEmpty());
    EXPECT_GE(errSpy.count(), 1);

    w.setMaxFileSizeBytes(100 * 1024);
    EXPECT_TRUE(w.loadFromFile(filePath));
    EXPECT_TRUE(w.lastError().isEmpty());
    EXPECT_TRUE(w.dataUrl().startsWith(QStringLiteral("data:image/")));
}

TEST(GrpcImagePickerWidgetTests, ClearUsesClearDataUrl)
{
    GrpcImagePickerWidget w;
    QSignalSpy spy(&w, &GrpcImagePickerWidget::dataUrlChanged);

    w.setClearDataUrl(QStringLiteral("data:image/png;base64,AAAA"));
    w.setDataUrl(QStringLiteral("data:image/png;base64,BBBB"));

    spy.clear();
    QMetaObject::invokeMethod(&w, "clearImage", Qt::DirectConnection);

    EXPECT_EQ(w.dataUrl(), QStringLiteral("data:image/png;base64,AAAA"));
    EXPECT_EQ(spy.count(), 1);
}
