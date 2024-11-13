#ifndef KIWIXWEBCHANNELOBJECT_H
#define KIWIXWEBCHANNELOBJECT_H

#include <QObject>

class KiwixWebChannelObject : public QObject
{
    Q_OBJECT

public:
    explicit KiwixWebChannelObject(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE void sendHeaders(const QJsonObject& headers) { emit headersChanged(headers); };

signals:
    void headersChanged(const QJsonObject& headers);
    void navigationRequested(const QString& url, const QString& anchor);
};

#endif // KIWIXWEBCHANNELOBJECT_H
