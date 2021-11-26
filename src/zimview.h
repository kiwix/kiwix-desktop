#ifndef ZIMVIEW_H
#define ZIMVIEW_H

#include <QWidget>
#include "findinpagebar.h"

class TabBar;
class WebView;

class ZimView : public QWidget
{
    Q_OBJECT
public:
    explicit ZimView(TabBar* tabBar, QWidget *parent = nullptr);

    WebView *getWebView() { return mp_webView; }
    FindInPageBar *getFindInPageBar() { return mp_findInPageBar; }
    void openFindInPageBar();

private:
    WebView *mp_webView;
    TabBar *mp_tabBar;
    FindInPageBar *mp_findInPageBar;
};

#endif // ZIMVIEW_H
