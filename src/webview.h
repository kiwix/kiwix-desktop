#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QIcon>
#include <QWheelEvent>

#include <kiwix/reader.h>
#include "findinpagebar.h"

class WebView : public QWebEngineView
{
    Q_OBJECT
    Q_PROPERTY(const QIcon icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString zimId READ zimId NOTIFY zimIdChanged)

public:
    WebView(QWidget *parent = Q_NULLPTR);
    virtual ~WebView();

    bool isWebActionEnabled(QWebEnginePage::WebAction webAction) const;
    const QIcon &icon() { return m_icon; }
    const QString &zimId() { return m_currentZimId; }

public slots:
    void onUrlChanged(const QUrl& url);

signals:
    void iconChanged(const QIcon& icon);
    void zimIdChanged(const QString& zimId);

protected:
    virtual QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
    void wheelEvent(QWheelEvent *event);
    bool event(QEvent *event);
    bool eventFilter(QObject *src, QEvent *e);
    void contextMenuEvent(QContextMenuEvent *event);

    QString m_currentZimId;
    QIcon m_icon;
    QString m_linkHovered;
};

#endif // WEBVIEW_H
