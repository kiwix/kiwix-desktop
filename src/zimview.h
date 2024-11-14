#ifndef ZIMVIEW_H
#define ZIMVIEW_H

#include <QWidget>
#include <QWebEnginePage>

class FindInPageBar;
class TabBar;
class WebView;

#if defined(QT_TEXTTOSPEECH_LIB)
class TextToSpeechBar;
#endif

class ZimView : public QWidget
{
    Q_OBJECT
public:
    explicit ZimView(TabBar* tabBar, QWidget *parent = nullptr);

    WebView *getWebView() { return mp_webView; }
    FindInPageBar *getFindInPageBar() { return mp_findInPageBar; }
#if defined(QT_TEXTTOSPEECH_LIB)
    TextToSpeechBar *getTextToSpeechBar() { return mp_ttsBar; }
#endif
    void openFindInPageBar();

public slots:
#if defined(QT_TEXTTOSPEECH_LIB)
    void readArticle();
    void readSelectedText();
    void setSpeechLocaleByZimId(const QString& zimId);
#endif

signals:
    void webActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);

private:
    WebView *mp_webView;
    TabBar *mp_tabBar;
    FindInPageBar *mp_findInPageBar;

#if defined(QT_TEXTTOSPEECH_LIB)
    TextToSpeechBar *mp_ttsBar;
#endif
};

#endif // ZIMVIEW_H
