#ifndef KIWIXWEBVIEW_H
#define KIWIXWEBVIEW_H

#include <QWebEngineView>
#include <kiwix/reader.h>

class KiwixWebView : public QWebEngineView
{
    Q_OBJECT

public:
    KiwixWebView(QWidget *parent = Q_NULLPTR);
    virtual ~KiwixWebView();

protected:
    virtual QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
};

#endif // KIWIXWEBVIEW_H
