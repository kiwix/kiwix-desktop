#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>

class WebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WebPage(QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

signals:

public slots:
    void startDownload(QWebEngineDownloadItem*);
};

#endif // WEBPAGE_H
