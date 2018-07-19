#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QIcon>

#include <kiwix/reader.h>

class WebView : public QWebEngineView
{
    Q_OBJECT
    Q_PROPERTY(const QIcon icon READ icon NOTIFY iconChanged);

public:
    WebView(QWidget *parent = Q_NULLPTR);
    virtual ~WebView();

    bool isWebActionEnabled(QWebEnginePage::WebAction webAction) const;
    const QIcon &icon() { return m_icon; }

public slots:
    void onUrlChanged(const QUrl& url);

signals:
    void iconChanged(const QIcon& icon);

protected:
    virtual QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
    QString m_currentHost;
    QIcon m_icon;
};

#endif // WEBVIEW_H
