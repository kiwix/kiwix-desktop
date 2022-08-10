#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QStackedWidget>
#include <memory>
#include "webview.h"
#include "zimview.h"
#include "contentmanagerview.h"
#include "fullscreenwindow.h"
#include <QMouseEvent>
#include <QWebEngineFullScreenRequest>

class TabBar : public QTabBar
{
    Q_OBJECT
public:
    TabBar(QWidget* parent=nullptr);
    void setStackedWidget(QStackedWidget* widget);

    void     setContentManagerView(ContentManagerView* view);
    void     setNewTabButton();
    ZimView* createNewTab(bool setCurrent, bool adjacentToCurrentTab);

    ZimView* currentZimView() {
        return qobject_cast<ZimView*>(mp_stackedWidget->currentWidget());
    }

    WebView* currentWebView() {
        if (ZimView *zv = currentZimView())
            return zv->getWebView();
        return nullptr;
    }

    void openUrl(const QUrl &url, bool newTab);
// Redirect call to sub-webView
    void setTitleOf(const QString& title, ZimView* tab=nullptr);
    void setIconOf(const QIcon& icon, ZimView* tab=nullptr);
    QString currentZimId();

    void triggerWebPageAction(QWebEnginePage::WebAction action, ZimView* widget=nullptr);
    QString currentArticleUrl();
    QString currentArticleTitle();
    virtual QSize tabSizeHint(int index) const;
    void openFindInPageBar();
    void closeTabsByZimId(const QString &id);

protected:
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void libraryPageDisplayed(bool displayed);
    void currentTitleChanged(const QString& title);

public slots:
    void closeTab(int index);
    void fullScreenRequested(QWebEngineFullScreenRequest request);
    void on_webview_titleChanged(const QString& title);

private:
    QStackedWidget*     mp_stackedWidget;
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;

    void setSelectionBehaviorOnRemove(int index);
    // The "+" (new tab) button is implemented as a tab (that is always placed at the end).
    // This function returns the count of real tabs.
    int realTabCount() const;
    
private slots:
    void onTabMoved(int from, int to);
    void onCurrentChanged(int index);
    void onWebviewHistoryActionChanged(QWebEnginePage::WebAction action, bool enabled);
};

#endif // TABWIDGET_H
