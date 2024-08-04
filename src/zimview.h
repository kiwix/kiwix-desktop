#ifndef ZIMVIEW_H
#define ZIMVIEW_H

#include <QWidget>
#include <QWebEnginePage>

class FindInPageBar;
#if defined(QT_TEXTTOSPEECH_LIB) && QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
class TextToSpeechBar;
#endif
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

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

private:
    WebView *mp_webView;
    TabBar *mp_tabBar;
    FindInPageBar *mp_findInPageBar;
#if defined(QT_TEXTTOSPEECH_LIB) && QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    TextToSpeechBar *mp_ttsBar;
#endif
};

#endif // ZIMVIEW_H
