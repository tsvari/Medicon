#include "GrpcImagePickerWidget.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

GrpcImagePickerWidget::GrpcImagePickerWidget(QWidget *parent)
    : QWidget(parent)
    , m_imageLabel(new QLabel(this))
    , m_showImageButton(new QPushButton(this))
    , m_clearImageButton(new QPushButton(this))
{
    m_imageLabel->setObjectName(QStringLiteral("imageLabel"));
    m_imageLabel->setFrameShape(QFrame::Box);
    m_imageLabel->setText(QString());
    m_imageLabel->setScaledContents(true);

    m_showImageButton->setObjectName(QStringLiteral("imageBrowseButton"));
    m_showImageButton->setText(tr("Show image..."));
    m_showImageButton->setVisible(m_showImageButtonVisible);

    connect(m_showImageButton, &QPushButton::clicked, this, &GrpcImagePickerWidget::chooseImage);

    m_clearImageButton->setObjectName(QStringLiteral("imageClearButton"));
    m_clearImageButton->setText(tr("Clear image"));
    m_clearImageButton->setVisible(m_clearImageButtonVisible);

    connect(m_clearImageButton, &QPushButton::clicked, this, &GrpcImagePickerWidget::clearImage);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_imageLabel);
    layout->addWidget(m_showImageButton);
    layout->addWidget(m_clearImageButton);

    setLayout(layout);
}

QLabel *GrpcImagePickerWidget::imageLabel() const
{
    return m_imageLabel;
}

QPushButton *GrpcImagePickerWidget::showImageButton() const
{
    return m_showImageButton;
}

QPushButton *GrpcImagePickerWidget::clearImageButton() const
{
    return m_clearImageButton;
}

QString GrpcImagePickerWidget::dataUrl() const
{
    return m_dataUrl;
}

void GrpcImagePickerWidget::setDataUrl(const QString &dataUrl)
{
    if (m_dataUrl == dataUrl) {
        return;
    }
    m_dataUrl = dataUrl;
    updatePreview();
    emit dataUrlChanged(m_dataUrl);
}

QString GrpcImagePickerWidget::clearDataUrl() const
{
    return m_clearDataUrl;
}

void GrpcImagePickerWidget::setClearDataUrl(const QString &dataUrl)
{
    if (m_clearDataUrl == dataUrl) {
        return;
    }
    m_clearDataUrl = dataUrl;
    emit clearDataUrlChanged(m_clearDataUrl);
}

bool GrpcImagePickerWidget::readOnly() const
{
    return m_readOnly;
}

void GrpcImagePickerWidget::setReadOnly(bool readOnly)
{
    if (m_readOnly == readOnly) {
        return;
    }
    m_readOnly = readOnly;

    if (m_showImageButton) {
        m_showImageButton->setEnabled(!m_readOnly);
    }
    if (m_clearImageButton) {
        m_clearImageButton->setEnabled(!m_readOnly);
    }
}

qint64 GrpcImagePickerWidget::maxFileSizeBytes() const
{
    return m_maxFileSizeBytes;
}

void GrpcImagePickerWidget::setMaxFileSizeBytes(qint64 bytes)
{
    if (bytes < 0) {
        bytes = 0;
    }
    if (m_maxFileSizeBytes == bytes) {
        return;
    }
    m_maxFileSizeBytes = bytes;
    emit maxFileSizeBytesChanged(m_maxFileSizeBytes);
}

QString GrpcImagePickerWidget::lastError() const
{
    return m_lastError;
}

bool GrpcImagePickerWidget::showImageButtonVisible() const
{
    return m_showImageButtonVisible;
}

void GrpcImagePickerWidget::setShowImageButtonVisible(bool visible)
{
    if (m_showImageButtonVisible == visible) {
        return;
    }
    m_showImageButtonVisible = visible;
    if (m_showImageButton) {
        m_showImageButton->setVisible(m_showImageButtonVisible);
    }
}

bool GrpcImagePickerWidget::clearImageButtonVisible() const
{
    return m_clearImageButtonVisible;
}

void GrpcImagePickerWidget::setClearImageButtonVisible(bool visible)
{
    if (m_clearImageButtonVisible == visible) {
        return;
    }
    m_clearImageButtonVisible = visible;
    if (m_clearImageButton) {
        m_clearImageButton->setVisible(m_clearImageButtonVisible);
    }
}

void GrpcImagePickerWidget::chooseImage()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose image"),
        QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.gif *.bmp *.tif *.tiff)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    loadFromFile(fileName);
}

bool GrpcImagePickerWidget::loadFromFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        setLastError(tr("No file selected."));
        return false;
    }

    const QFileInfo fileInfo(fileName);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setLastError(tr("File not found."));
        return false;
    }

    if (m_maxFileSizeBytes > 0 && fileInfo.size() > m_maxFileSizeBytes) {
        setLastError(tr("Image is too large (%1). Max allowed is %2.")
                         .arg(humanFileSize(fileInfo.size()), humanFileSize(m_maxFileSizeBytes)));
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        setLastError(tr("Cannot open file."));
        return false;
    }

    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        setLastError(tr("File is empty."));
        return false;
    }

    if (m_maxFileSizeBytes > 0 && bytes.size() > m_maxFileSizeBytes) {
        setLastError(tr("Image is too large (%1). Max allowed is %2.")
                         .arg(humanFileSize(bytes.size()), humanFileSize(m_maxFileSizeBytes)));
        return false;
    }

    const QString suffixLower = fileInfo.suffix().toLower();
    setLastError(QString());
    setDataUrl(bytesToDataUrl(bytes, suffixLower));
    return true;
}

void GrpcImagePickerWidget::clearImage()
{
    setLastError(QString());
    setDataUrl(m_clearDataUrl);
}

void GrpcImagePickerWidget::updatePreview()
{
    if (!m_imageLabel) {
        return;
    }

    const QString s = m_dataUrl;
    if (s.isEmpty()) {
        m_imageLabel->setPixmap(QPixmap());
        m_imageLabel->setText(QString());
        return;
    }

    static const QString dataPrefix = QStringLiteral("data:image/");
    static const QString base64Marker = QStringLiteral(";base64,");
    if (s.startsWith(dataPrefix)) {
        const int markerPos = s.indexOf(base64Marker);
        if (markerPos > 0) {
            const int payloadStart = markerPos + base64Marker.size();
            const QByteArray b64 = s.mid(payloadStart).toLatin1();
            const QByteArray bytes = QByteArray::fromBase64(b64);
            QPixmap pix;
            if (pix.loadFromData(bytes)) {
                m_imageLabel->setPixmap(pix);
                m_imageLabel->setText(QString());
                return;
            }
        }
    }

    QPixmap pix(s);
    if (!pix.isNull()) {
        m_imageLabel->setPixmap(pix);
        m_imageLabel->setText(QString());
        return;
    }

    m_imageLabel->setPixmap(QPixmap());
    m_imageLabel->setText(s);
}

QString GrpcImagePickerWidget::bytesToDataUrl(const QByteArray &bytes, const QString &fileSuffixLower)
{
    const QString mime = (fileSuffixLower == QStringLiteral("png"))
        ? QStringLiteral("image/png")
        : (fileSuffixLower == QStringLiteral("webp"))
            ? QStringLiteral("image/webp")
            : (fileSuffixLower == QStringLiteral("gif"))
                ? QStringLiteral("image/gif")
                : (fileSuffixLower == QStringLiteral("bmp"))
                    ? QStringLiteral("image/bmp")
                    : ((fileSuffixLower == QStringLiteral("tif")) || (fileSuffixLower == QStringLiteral("tiff")))
                        ? QStringLiteral("image/tiff")
                        : QStringLiteral("image/jpeg");

    return QStringLiteral("data:%1;base64,%2")
        .arg(mime, QString::fromLatin1(bytes.toBase64()));
}

QString GrpcImagePickerWidget::humanFileSize(qint64 bytes)
{
    if (bytes < 0) {
        return QString();
    }
    if (bytes < 1024) {
        return tr("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024) {
        return tr("%1 KB").arg(QString::number(bytes / 1024.0, 'f', 1));
    }
    return tr("%1 MB").arg(QString::number(bytes / (1024.0 * 1024.0), 'f', 1));
}

void GrpcImagePickerWidget::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged(m_lastError);
    if (!m_lastError.isEmpty() && m_imageLabel) {
        m_imageLabel->setPixmap(QPixmap());
        m_imageLabel->setText(m_lastError);
    }
}
