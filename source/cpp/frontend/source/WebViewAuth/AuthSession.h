#ifndef AUTHSESSION_H
#define AUTHSESSION_H

#include <QObject>

class AuthSession : public QObject
{
    Q_OBJECT
public:
    explicit AuthSession(QObject *parent = nullptr);

signals:
};

#endif // AUTHSESSION_H
