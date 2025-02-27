#ifndef TABLEOFCONTENTBAR_H
#define TABLEOFCONTENTBAR_H

#include <QFrame>
#include <QTimer>

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
    void updateSelectionFromFragment(const QString& fragment);

signals:
    void navigationRequested(const QString& url, const QString& anchor);

private:
    Ui::tableofcontentbar *ui;
    QString m_url;
    QTimer m_clickDebounceTimer;
    bool m_isNavigating = false;
    QString m_lastAnchor;
};

#endif // TABLEOFCONTENTBAR_H