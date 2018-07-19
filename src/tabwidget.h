#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTableWidget>
#include <memory>
#include <kiwix/reader.h>
#include "webview.h"

class TabWidget : public QTabWidget
{
public:
    TabWidget(QWidget* parent=nullptr);

    WebView* createNewTab(bool setCurrent);
    void openUrl(std::shared_ptr<kiwix::Reader> reader, const QUrl &url, bool newTab);
    void setTitleOf(WebView* webView, const QString& title);
    void setIconOf(WebView* webView, const QIcon& icon);
};

#endif // TABWIDGET_H
