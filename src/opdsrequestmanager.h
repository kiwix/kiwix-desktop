#ifndef OPDSREQUESTMANAGER_H
#define OPDSREQUESTMANAGER_H

#include <QObject>
#include <QtNetwork>
#include <kiwix/library.h>
#include <kiwix/manager.h>

class OpdsRequestManager : public QObject
{
    Q_OBJECT

public:

    OpdsRequestManager();
    ~OpdsRequestManager() {}

public:
    void doUpdate(const QString& currentLanguage, const QString& categoryFilter);

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply *mp_reply;

signals:
    void requestReceived(const QString&);

public slots:
    void receiveContent();
};

#endif // OPDSREQUESTMANAGER_H