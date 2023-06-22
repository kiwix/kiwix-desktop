#ifndef CONTENTMANAGERVIEW_H
#define CONTENTMANAGERVIEW_H

#include <QWebEngineView>
#include <QTreeView>

class ContentManagerView : public QTreeView
{
    Q_OBJECT
public:
    ContentManagerView(QWidget *parent = Q_NULLPTR);
};

#endif // CONTENTMANAGERVIEW_H
