#pragma once

#include <QWidget>

#include <QString>

class QLabel;
class QPushButton;

/**
 * @file GrpcImagePickerWidget.h
 * @brief Image picker/preview widget storing the value as a data URL string.
 *
 * This widget is intended to integrate with the form/binding layer by exposing a
 * single string property: @ref GrpcImagePickerWidget::dataUrl.
 *
 * Typical stored value is a data URL:
 * - `data:image/<mime>;base64,<payload>`
 *
 * The preview logic is tolerant:
 * - If the string looks like a `data:image/...;base64,...` URL, it decodes and
 *   previews it.
 * - Otherwise it tries to treat the string as a file path.
 * - If that fails, it shows the string as plain text in the label.
 *
 * Image selection enforces @ref GrpcImagePickerWidget::maxFileSizeBytes (default
 * 100 KiB). When an error occurs, @ref GrpcImagePickerWidget::lastError is set,
 * and the label shows the error message.
 *
 * Note: This is a QWidget and must be used from the UI thread.
 */
class GrpcImagePickerWidget final : public QWidget
{
    Q_OBJECT

    /**
     * @brief The current image value.
     *
     * Usually a `data:image/...;base64,...` URL. Empty means "no image".
     * Setting this updates the preview and emits @ref dataUrlChanged.
     */
    Q_PROPERTY(QString dataUrl READ dataUrl WRITE setDataUrl NOTIFY dataUrlChanged)

    /**
     * @brief Value assigned when the user clicks "Clear image".
     *
     * If empty, "Clear" removes the image.
     */
    Q_PROPERTY(QString clearDataUrl READ clearDataUrl WRITE setClearDataUrl NOTIFY clearDataUrlChanged)

    /**
     * @brief Disables user interaction.
     *
     * When true, the "Show image..." and "Clear image" buttons are disabled.
     * The preview remains visible.
     */
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)

    /**
     * @brief Maximum allowed on-disk file size in bytes.
     *
     * Applied by @ref chooseImage and @ref loadFromFile.
     * - 0 means "no limit".
     * - Negative values are treated as 0.
     */
    Q_PROPERTY(qint64 maxFileSizeBytes READ maxFileSizeBytes WRITE setMaxFileSizeBytes NOTIFY maxFileSizeBytesChanged)

    /**
     * @brief Last user-visible error.
     *
     * Set when a load fails (file not found, too large, unreadable, etc).
     * Empty indicates no error.
     */
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

    /**
     * @brief Controls whether the "Show image..." button is shown.
     *
     * This property reflects the configured visibility (default true), not the
     * runtime QWidget visibility which may be false before the widget is shown.
     */
    Q_PROPERTY(bool showImageButtonVisible READ showImageButtonVisible WRITE setShowImageButtonVisible)

    /**
     * @brief Controls whether the "Clear image" button is shown.
     *
     * This property reflects the configured visibility (default true), not the
     * runtime QWidget visibility which may be false before the widget is shown.
     */
    Q_PROPERTY(bool clearImageButtonVisible READ clearImageButtonVisible WRITE setClearImageButtonVisible)

public:
    /**
     * @brief Constructs the widget.
     * @param parent Parent widget.
     */
    explicit GrpcImagePickerWidget(QWidget *parent = nullptr);

    /**
     * @brief Loads an image from disk.
     *
     * Applies @ref maxFileSizeBytes and, on success, updates @ref dataUrl to a
     * `data:image/...;base64,...` URL and refreshes the preview.
     *
     * @return true on success.
     * @return false on failure; in this case @ref lastError is set and @ref dataUrl
     * is unchanged.
     */
    Q_INVOKABLE bool loadFromFile(const QString &fileName);

    /**
     * @brief Returns the internal label that shows the preview or error text.
     *
     * Provided for customization and for legacy code that expects a label.
     */
    QLabel *imageLabel() const;

    /** @brief Returns the "Show image..." button. */
    QPushButton *showImageButton() const;

    /** @brief Returns the "Clear image" button. */
    QPushButton *clearImageButton() const;

    /** @brief Returns @ref dataUrl. */
    QString dataUrl() const;
    /** @brief Sets @ref dataUrl. */
    void setDataUrl(const QString &dataUrl);

    /** @brief Returns @ref clearDataUrl. */
    QString clearDataUrl() const;
    /** @brief Sets @ref clearDataUrl. */
    void setClearDataUrl(const QString &dataUrl);

    /** @brief Returns @ref readOnly. */
    bool readOnly() const;
    /** @brief Sets @ref readOnly. */
    void setReadOnly(bool readOnly);

    /** @brief Returns @ref maxFileSizeBytes. */
    qint64 maxFileSizeBytes() const;
    /** @brief Sets @ref maxFileSizeBytes. */
    void setMaxFileSizeBytes(qint64 bytes);

    /** @brief Returns @ref lastError. */
    QString lastError() const;

    /** @brief Returns @ref showImageButtonVisible. */
    bool showImageButtonVisible() const;
    /** @brief Sets @ref showImageButtonVisible. */
    void setShowImageButtonVisible(bool visible);

    /** @brief Returns @ref clearImageButtonVisible. */
    bool clearImageButtonVisible() const;
    /** @brief Sets @ref clearImageButtonVisible. */
    void setClearImageButtonVisible(bool visible);

signals:
    /** @brief Emitted when @ref dataUrl changes. */
    void dataUrlChanged(const QString &dataUrl);
    /** @brief Emitted when @ref clearDataUrl changes. */
    void clearDataUrlChanged(const QString &dataUrl);
    /** @brief Emitted when @ref maxFileSizeBytes changes. */
    void maxFileSizeBytesChanged(qint64 bytes);
    /** @brief Emitted when @ref lastError changes. */
    void lastErrorChanged(const QString &error);

private slots:
    void chooseImage();
    void clearImage();

private:
    void updatePreview();
    static QString bytesToDataUrl(const QByteArray &bytes, const QString &fileSuffixLower);
    static QString humanFileSize(qint64 bytes);
    void setLastError(const QString &error);

    QLabel *m_imageLabel;
    QPushButton *m_showImageButton;
    QPushButton *m_clearImageButton;

    QString m_dataUrl;
    QString m_clearDataUrl;

    bool m_showImageButtonVisible = true;
    bool m_clearImageButtonVisible = true;

    bool m_readOnly = false;

    qint64 m_maxFileSizeBytes = 100 * 1024;
    QString m_lastError;
};
