#include "tab.h"
#include <QVBoxLayout>

Tab::Tab(WebView* p_webView, QWidget *parent)
    : QWidget(parent),
      mp_webView(p_webView),
      mp_findInPageBar(new FindInPageBar(this))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mp_webView);
    layout->addWidget(mp_findInPageBar);
    setLayout(layout);
}
