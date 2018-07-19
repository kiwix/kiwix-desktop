#ifndef KIWIXWEBVIEW_H
#define KIWIXWEBVIEW_H

#include <QWebEngineView>
#include <QIcon>

#include <kiwix/reader.h>

class KiwixWebView : public QWebEngineView
{
    Q_OBJECT
    Q_PROPERTY(const QIcon icon READ icon NOTIFY iconChanged);

public:
    KiwixWebView(QWidget *parent = Q_NULLPTR);
    virtual ~KiwixWebView();

    const QIcon &icon() { return _icon; }

public slots:
    void onUrlChanged(const QUrl& url);

signals:
    void iconChanged(const QIcon& icon);

protected:
    virtual QWebEngineView* createWindow(QWebEnginePage::WebWindowType type);
    QString currentHost;
    QIcon _icon;
};

#endif // KIWIXWEBVIEW_H
