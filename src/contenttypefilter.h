#ifndef CONTENTTYPEFILTER_H
#define CONTENTTYPEFILTER_H

#include <QCheckBox>
#include <QMap>

class ContentTypeFilter : public QCheckBox
{
    Q_OBJECT

public:
    explicit ContentTypeFilter(QString name, QWidget *parent = nullptr);
    virtual ~ContentTypeFilter() {}

    QString getName() { return m_name; }

public slots:
    void onStateChanged(int state);

private:
    QString m_name;
    QMap<Qt::CheckState, QString> m_states;
};

#endif // CONTENTTYPEFILTER_H
