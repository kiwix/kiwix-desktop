#ifndef TABLEOFCONTENTBAR_H
#define TABLEOFCONTENTBAR_H

#include <QFrame>

namespace Ui {
class tableofcontentbar;
}

class QTreeWidgetItem;

class TableOfContentBar : public QFrame
{
    Q_OBJECT

public:
    explicit TableOfContentBar(QWidget *parent = nullptr);
    ~TableOfContentBar();

public slots:
    void setupTree(const QJsonObject& headers);
    void onTreeItemActivated(QTreeWidgetItem* item);

signals:
    void navigationRequested(const QString& url, const QString& anchor);

private:
    Ui::tableofcontentbar *ui;
    QString m_url;
};

#endif // TABLEOFCONTENTBAR_H
