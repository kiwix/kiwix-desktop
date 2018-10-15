#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTableWidget>
#include <memory>
#include "webview.h"
#include "contentmanagerview.h"

class TabWidget : public QTabWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentZimId READ currentZimId NOTIFY currentZimIdChanged)
public:
    TabWidget(QWidget* parent=nullptr);

    void     setContentManagerView(ContentManagerView* view);
    WebView* createNewTab(bool setCurrent);
    WebView* widget(int index) { return (index != 0) ? static_cast<WebView*>(QTabWidget::widget(index)) : nullptr; }
    WebView* currentWidget() { auto current = QTabWidget::currentWidget();
                               if (current == mp_contentManagerView) return nullptr;
                               return static_cast<WebView*>(current);
                             }

    void openUrl(const QUrl &url, bool newTab);
// Redirect call to sub-webView
    void setTitleOf(const QString& title, WebView* webView=nullptr);
    void setIconOf(const QIcon& icon, WebView* webView=nullptr);
    QString currentZimId();

    void triggerWebPageAction(QWebEnginePage::WebAction action, WebView* webView=nullptr);
signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void currentZimIdChanged(const QString& zimId);

public slots:
    void closeTab(int index);
    void onCurrentChanged(int index);

private:
    ContentManagerView* mp_contentManagerView;

};

#endif // TABWIDGET_H
