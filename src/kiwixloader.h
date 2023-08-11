#ifndef KIWIXLOADER_H
#define KIWIXLOADER_H

#include <QWidget>
#include <QTimer>

class KiwixLoader : public QWidget
{
    Q_OBJECT

public:
    explicit KiwixLoader(QWidget *parent = nullptr);
    ~KiwixLoader();
    void startAnimation();
    void stopAnimation();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateAnimation();

private:
    QTimer *m_timer;
    int progress = 0;
};

#endif // KIWIXLOADER_H
