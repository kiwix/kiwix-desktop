#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include "webview.h"
#include "findinpagebar.h"

class Tab : public QWidget
{
    Q_OBJECT
public:
    explicit Tab(WebView* p_webView, QWidget *parent = nullptr);

    WebView *getWebView() { return mp_webView; }
    FindInPageBar *getFindInPageBar() { return mp_findInPageBar; }


signals:

private:
    WebView *mp_webView;
    FindInPageBar *mp_findInPageBar;

};

#endif // TAB_H
