#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QMenu>
#include <QWebEngineView>
#include <QIcon>
#include <QWheelEvent>
#include <QJsonObject>

#include "findinpagebar.h"

class QWebEngineHistoryItem;


class WebViewBackMenu : public QMenu
{
    Q_OBJECT
public:
    WebViewBackMenu(QWidget* parent=nullptr) : QMenu(parent) {}
    void showEvent(QShowEvent *);
};

class WebViewForwardMenu : public QMenu
{
    Q_OBJECT
public:
    WebViewForwardMenu(QWidget* parent=nullptr) : QMenu(parent) {}
    void showEvent(QShowEvent *);
};


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

    QMenu* getHistoryBackMenu() const;
    QMenu* getHistoryForwardMenu() const;

    void saveViewContent();

public slots:
    void onUrlChanged(const QUrl& url);

signals:
    void iconChanged(const QIcon& icon);
    void zimIdChanged(const QString& zimId);
    void headersChanged(const QJsonObject& headers);
    void navigationRequested(const QString& url, const QString& anchor);

protected:
    virtual QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
    void wheelEvent(QWheelEvent *event);
    bool event(QEvent *event);
    bool eventFilter(QObject *src, QEvent *e);
    void contextMenuEvent(QContextMenuEvent *event);

    QString m_currentZimId;
    QIcon m_icon;
    QString m_linkHovered;

private slots:
    void gotoTriggeredHistoryItemAction();
    void onCurrentTitleChanged();
    void onHeadersReceived(const QString& headersJSONStr);
    void onConsoleMessageReceived(const QString& message);
    void onNavigationRequested(const QString& url, const QString& anchor);
    void handleTocHistoryNavigation(const QUrl& url);

private:
    void addHistoryItemAction(QMenu *menu, const QWebEngineHistoryItem &item, int n) const;
    void applyCorrectZoomFactor();
    QMenu* createStandardContextMenu();
    QMenu* createLinkContextMenu();
    QJsonObject m_headers;
};

#endif // WEBVIEW_H