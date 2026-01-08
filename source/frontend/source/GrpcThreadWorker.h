#ifndef GRPCTHREADWORKER_H
#define GRPCTHREADWORKER_H

#include <QEvent>
#include <QLabel>
#include <QMovie>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QWidget>

class GrpcLoader : public QLabel
{
    Q_OBJECT

private:
    /**
     * @brief Margin in pixels used for top-aligned placement.
     */
    int const animationMargin = 4;
public:
    /**
     * @brief Placement of the loader within its parent widget.
     */
    enum Direction {Center = 0, TopCenter, TopRight};

    /**
     * @brief Creates an animated loader label backed by a QMovie.
     *
     * The loader is parented to @p parent and will attempt to keep itself aligned:
     * - when the movie advances frames
     * - when the parent widget resizes/moves/layouts (via an event filter)
     *
     * Typical usage:
     * @code
     * auto * loader = new GrpcLoader(":/icons/loaderSmall.gif", GrpcLoader::Center, someParent);
     * loader->showLoader(true);
     * @endcode
     *
     * @param fileName Resource or filesystem path to an animated image (e.g. GIF).
     * @param direction Placement within the parent.
     * @param parent Parent widget that will contain the loader.
     */
    explicit GrpcLoader(const QString & fileName, Direction direction, QWidget * parent = nullptr)
        : QLabel(parent)
        , movie(new QMovie(fileName, {}, this))
        , direction(direction)
    {
        setMovie(movie);

        // Keep the loader aligned when the movie advances or the parent resizes.
        connect(movie, &QMovie::frameChanged, this, &GrpcLoader::adjust);

        if (auto * pw = parentWidget()) {
            pw->installEventFilter(this);
        }
    }

    /**
     * @brief Calculates the top-left position for a content rectangle inside a parent.
     *
     * This helper is pure and can be unit-tested without constructing widgets.
     *
     * @param parentSize Size of the available parent area.
     * @param contentSize Size of the content (movie frame/pixmap).
     * @param direction Desired placement.
     * @param margin Margin in pixels used for top-aligned placements.
     * @return Top-left position of the content within the parent.
     */
    static QPoint calculatePosition(const QSize & parentSize,
                                   const QSize & contentSize,
                                   Direction direction,
                                   int margin)
    {
        const int xCenter = (parentSize.width() - contentSize.width()) / 2;
        const int yCenter = (parentSize.height() - contentSize.height()) / 2;

        switch (direction) {
        case Center:
            return QPoint(xCenter, yCenter);
        case TopCenter:
            return QPoint(xCenter, margin);
        case TopRight:
            return QPoint(parentSize.width() - contentSize.width() - margin, margin);
        }

        // Defensive fallback.
        return QPoint(xCenter, yCenter);
    }

    /**
     * @brief Starts or stops the animation and shows/hides the loader.
     *
     * When showing, this also triggers an immediate positioning update.
     *
     * @param showLoader If true, shows the label and starts the movie; otherwise stops and hides.
     */
    void showLoader(bool showLoader)
    {
        if(showLoader) {
            show();
            movie->start();
            adjust();
        } else {
            movie->stop();
            hide();
        }
    }

protected:
    /**
     * @brief Watches parent widget events to keep the loader aligned.
     *
     * The loader repositions itself when visible and the parent changes geometry/layout.
     */
    bool eventFilter(QObject * watched, QEvent * event) override
    {
        if (watched == parentWidget()) {
            switch (event->type()) {
            case QEvent::Resize:
            case QEvent::Move:
            case QEvent::LayoutRequest:
            case QEvent::Show:
                if (isVisible()) {
                    adjust();
                }
                break;
            default:
                break;
            }
        }
        return QLabel::eventFilter(watched, event);
    }

private slots:
    /**
     * @brief Recomputes size and position based on current movie frame and parent size.
     *
     * This is triggered automatically from QMovie frame changes and parent events.
     * If the movie is not running or no valid frame size is available, this is a no-op.
     */
    void adjust()
    {
        auto * pw = parentWidget();
        if (!pw || !movie) {
            return;
        }

        if (movie->state() != QMovie::Running) {
            return;
        }

        QSize contentSize = movie->frameRect().size();
        if (contentSize.isEmpty()) {
            contentSize = movie->currentPixmap().size();
        }
        if (contentSize.isEmpty()) {
            return;
        }

        resize(contentSize);
        move(calculatePosition(pw->rect().size(), contentSize, direction, animationMargin));
    }

private:
    /**
     * @brief Animation source for the loader.
     *
     * Owned by this widget via QObject parent/child hierarchy.
     */
    QMovie * movie = nullptr;

    /**
     * @brief Placement direction used by adjust().
     */
    Direction direction;
};


#endif // GRPCTHREADWORKER_H
