#ifndef CONTENTMANAGERVIEW_H
#define CONTENTMANAGERVIEW_H

#include <QWebEngineView>
#include <QWebChannel>

class ContentManagerView : public QWebEngineView
{
public:
    ContentManagerView(QWidget *parent = Q_NULLPTR);
    void registerObject(const QString &id, QObject *object);
    void setHtml();
private:
    QWebChannel m_webChannel;
};

#endif // CONTENTMANAGERVIEW_H
