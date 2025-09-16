#ifndef GRPCTHREADWORKER_H
#define GRPCTHREADWORKER_H

#include <QObject>
#include <QLabel>
#include <QMovie>

class IBaseDataContainer;
class GrpcLoader : public QLabel
{
    Q_OBJECT

private:
    int const animationMargin = 4;
public:
    enum Direction {Center = 0, TopCenter, TopRight};
    explicit GrpcLoader(const QString & fileName, Direction direction, QWidget * parent = nullptr)
        : QLabel(parent)
        , direction(direction)
    {
        movie = new QMovie(fileName, {}, this);
        setMovie(movie);
        connect(this, &GrpcLoader::adjustToParentWidget, this, &GrpcLoader::adjust);
    }

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

signals:
    void adjustToParentWidget();

private slots:
    void adjust()
    {
        QRect parentRect = qobject_cast<QWidget*>(parent())->geometry();
        if(movie->state() == QMovie::Running) {
            QRect movieRect = movie->frameRect();
            resize(QSize(movieRect.size()));
            if(direction == Center) {
                move(QPoint(
                    (parentRect.width() - movieRect.width()) / 2,
                    (parentRect.height() - movieRect.height()) / 2));
            } else if(direction == TopCenter) {
                move(QPoint((parentRect.width() - movieRect.width()) / 2, animationMargin));
            } else if(direction == TopRight) {
                move(QPoint((parentRect.width() - movieRect.width()) / 2 - animationMargin, animationMargin));
            }

        }
    }

private:
    QMovie * movie;
    Direction direction;
};


class GrpcThreadWorker : public QObject
{
    Q_OBJECT
public:
    explicit GrpcThreadWorker(QObject * parent = nullptr);

    // Pure Virtual functions to load, add, edit, delete
    // Make real implementations in classes derived from GrpcThreadWorker
    virtual IBaseDataContainer * loadObjects() = 0;
    virtual void addNewObject(const QVariant & promise) = 0;
    virtual void editObject(const QVariant & promise) = 0;
    virtual void deleteObject(const QVariant & promise) = 0;
};

#endif // GRPCTHREADWORKER_H
