#ifndef KIWIXWEBVIEW_H
#define KIWIXWEBVIEW_H

#include <QWebEngineView>
#include "kiwixschemehandler.h"
#include "kiwixrequestinterceptor.h"


class KiwixWebView : public QWebEngineView
{
    Q_OBJECT

public:
    KiwixWebView(QWidget *parent = Q_NULLPTR);
    virtual ~KiwixWebView();

private:
    KiwixSchemeHandler schemeHandler;
    KiwixRequestInterceptor requestInterceptor;
};

#endif // KIWIXWEBVIEW_H
