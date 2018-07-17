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

    void initFromReader(std::shared_ptr<kiwix::Reader> reader);
};

#endif // KIWIXWEBVIEW_H
