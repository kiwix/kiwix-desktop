#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QUrl>

class WebPage : public QWebEnginePage
{
	Q_OBJECT
public:
    explicit WebPage(QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private slots:
    void handleFeaturePermissionRequested(const QUrl &origin, Feature feature);
};

#endif // WEBPAGE_H
