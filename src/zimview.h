#ifndef ZIMVIEW_H
#define ZIMVIEW_H

#include <QWidget>
#include <QWebEnginePage>

class FindInPageBar;
class TabBar;
class WebView;
class TextToSpeechBar;

class ZimView : public QWidget
{
    Q_OBJECT
public:
    explicit ZimView(TabBar* tabBar, QWidget *parent = nullptr);

    WebView *getWebView() { return mp_webView; }
    FindInPageBar *getFindInPageBar() { return mp_findInPageBar; }
    void openFindInPageBar();

public slots:
    void readArticle();
    void readSelectedText();

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

private:
    WebView *mp_webView;
    TabBar *mp_tabBar;
    FindInPageBar *mp_findInPageBar;
    TextToSpeechBar *mp_ttsBar;
};

#endif // ZIMVIEW_H
