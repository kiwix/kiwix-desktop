#ifndef KIWIXWEBCHANNELOBJECT_H
#define KIWIXWEBCHANNELOBJECT_H

#include <QObject>

class KiwixWebChannelObject : public QObject
{
    Q_OBJECT
public:
    explicit KiwixWebChannelObject(QObject *parent = nullptr) : QObject(parent) {};

    Q_INVOKABLE QString getTocTitle() const;
    Q_PROPERTY(QString tocTitle READ getTocTitle CONSTANT);

    Q_INVOKABLE bool getTocVisible() const;
    Q_PROPERTY(bool tocVisible READ getTocVisible NOTIFY tocVisibleChanged);

signals:
    void tocVisibleChanged(bool visible);
};

#endif // KIWIXWEBCHANNELOBJECT_H
