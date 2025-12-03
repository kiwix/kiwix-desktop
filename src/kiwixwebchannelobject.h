#ifndef KIWIXWEBCHANNELOBJECT_H
#define KIWIXWEBCHANNELOBJECT_H

#include <QObject>

class KiwixWebChannelObject : public QObject
{
    Q_OBJECT

public:
    explicit KiwixWebChannelObject(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE void sendHeadersJSONStr(const QString& headersJSONStr) { emit headersChanged(headersJSONStr); };
    Q_INVOKABLE void sendConsoleMessage(const QString& message) { emit consoleMessageReceived(message); };

signals:
    void headersChanged(const QString& headersJSONStr);
    void navigationRequested(const QString& url, const QString& anchor);
    void consoleMessageReceived(const QString& message);
};

#endif // KIWIXWEBCHANNELOBJECT_H