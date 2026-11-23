#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QUrl>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QWebEnginePermission>
#endif

class WebPage : public QWebEnginePage
{
	Q_OBJECT
public:
    explicit WebPage(QObject *parent = nullptr);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

private slots:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    void handlePermissionRequested(const QWebEnginePermission &permission);
#else
    void handleFeaturePermissionRequested(const QUrl &origin, Feature feature);
#endif
};

#endif // WEBPAGE_H
