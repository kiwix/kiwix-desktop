#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QStackedWidget>
#include <memory>
#include "webview.h"
#include "contentmanagerview.h"

class TabBar : public QTabBar
{
    Q_OBJECT
    Q_PROPERTY(QString currentZimId READ currentZimId NOTIFY currentZimIdChanged)

public:
    TabBar(QWidget* parent=nullptr);
    void setStackedWidget(QStackedWidget* widget);

    void     setContentManagerView(ContentManagerView* view);
    void     setNewTabButton();
    WebView* createNewTab(bool setCurrent);
    WebView* widget(int index) { return (index != 0) ? static_cast<WebView*>(mp_stackedWidget->widget(index)) : nullptr; }
    WebView* currentWidget() { auto current = mp_stackedWidget->currentWidget();
                               if (current == mp_contentManagerView) return nullptr;
                               return static_cast<WebView*>(current);
                             }

    void openUrl(const QUrl &url, bool newTab);
// Redirect call to sub-webView
    void setTitleOf(const QString& title, WebView* webView=nullptr);
    void setIconOf(const QIcon& icon, WebView* webView=nullptr);
    QString currentZimId();

    void triggerWebPageAction(QWebEnginePage::WebAction action, WebView* webView=nullptr);
    QString currentArticleUrl();
    QString currentArticleTitle();
    virtual QSize tabSizeHint(int index) const;
signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void currentZimIdChanged(const QString& zimId);
    void currentTitleChanged(const QString& title);

public slots:
    void closeTab(int index);
    void onCurrentChanged(int index);

private:
    ContentManagerView* mp_contentManagerView;
    QStackedWidget*     mp_stackedWidget;

    void setSelectionBehaviorOnRemove(int index);

};

#endif // TABWIDGET_H
