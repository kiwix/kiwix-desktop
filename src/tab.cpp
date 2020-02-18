#include "tab.h"
#include <QVBoxLayout>

Tab::Tab(QWidget *parent)
    : QWidget(parent),
      mp_webView(nullptr)
      ,
      mp_findInPageBar(new FindInPageBar(this))
{
    // QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget(mp_webView);
    // // layout->addWidget(mp_findInPageBar);
    // setLayout(layout);
}
