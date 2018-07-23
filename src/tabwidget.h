#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTableWidget>
#include <memory>
#include "webview.h"

class TabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentZimId READ currentZimId NOTIFY currentZimIdChanged)
public:
    TabWidget(QWidget* parent=nullptr);

    WebView* createNewTab(bool setCurrent);
    WebView* widget(int index) { return static_cast<WebView*>(QTabWidget::widget(index)); }
    WebView* currentWidget() { return static_cast<WebView*>(QTabWidget::currentWidget()); }

    void openUrl(const QUrl &url, bool newTab);
// Redirect call to sub-webView
    void setTitleOf(const QString& title, WebView* webView=nullptr);
    void setIconOf(const QIcon& icon, WebView* webView=nullptr);
    const QString& currentZimId();

    void triggerWebPageAction(QWebEnginePage::WebAction action, WebView* webView=nullptr);
signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void currentZimIdChanged(const QString& zimId);

public slots:
    void closeTab(int index);
    void onCurrentChanged(int index);
};

#endif // TABWIDGET_H
