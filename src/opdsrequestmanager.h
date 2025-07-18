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
    QNetworkReply* opdsResponseFromUrl(const QUrl &url);

private:
    QNetworkAccessManager m_networkManager;
    QNetworkReply* opdsResponseFromPath(const QString &path, const QUrlQuery &query = QUrlQuery());
    void handleNetworkReply(QNetworkReply* reply, void (OpdsRequestManager::*finalHandler)(QNetworkReply*), int redirectCount);
    void handleReply(QNetworkReply* reply, std::function<void(QNetworkReply*)> finalHandler, int redirectCount);
    static constexpr int MAX_REDIRECTS = 5;

signals:
    void requestReceived(const QString&);
    void languagesReceived(const QString&);
    void categoriesReceived(const QString&);
    void requestError(const QString& errorMessage);

public slots:
    void receiveContent(QNetworkReply*);
    void receiveLanguages(QNetworkReply*);
    void receiveCategories(QNetworkReply*);

public:
    static QString getCatalogHost();
    static int     getCatalogPort();
};

#endif // OPDSREQUESTMANAGER_H


