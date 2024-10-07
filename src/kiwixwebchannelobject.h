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

    Q_INVOKABLE QString getHideButtonText() const;
    Q_PROPERTY(QString hideButtontext READ getHideButtonText CONSTANT);

    Q_INVOKABLE bool getTocVisible() const;
    Q_INVOKABLE void setTocVisible(bool visible);
    Q_PROPERTY(bool tocVisible READ getTocVisible WRITE setTocVisible NOTIFY tocVisibleChanged);

signals:
    void tocVisibleChanged(bool visible);
};

#endif // KIWIXWEBCHANNELOBJECT_H
