#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QToolBar>
#include <QLineEdit>

class TopWidget : public QToolBar
{
    Q_OBJECT
public:
    explicit TopWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    QLineEdit searchEntry;
    QAction* fullScreenAction;
    QAction* normalScreenAction;
    bool fullScreen;
    QPoint m_pCursor;
    ulong timestamp;

protected slots:
    void toggleFullScreen();
};

#endif // TOPWIDGET_H
