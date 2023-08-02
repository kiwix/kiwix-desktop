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
    void getLanguagesFromOpds();
    void getCategoriesFromOpds();

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply* opdsResponseFromPath(const QString &path, const QUrlQuery &query = QUrlQuery());

signals:
    void requestReceived(const QString&);
    void languagesReceived(const QString&);
    void categoriesReceived(const QString&);

public slots:
    void receiveContent(QNetworkReply*);
    void receiveLanguages(QNetworkReply*);
    void receiveCategories(QNetworkReply*);
};

#endif // OPDSREQUESTMANAGER_H
