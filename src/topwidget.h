#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QToolBar>
#include <QLineEdit>
#include <QWebEnginePage>

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
    QAction* m_historyBackAction;
    QAction* m_historyForwardAction;
    QAction* fullScreenAction;
    QAction* normalScreenAction;
    bool fullScreen;
    QPoint m_pCursor;
    ulong timestamp;

protected slots:
    void toggleFullScreen();

private slots:
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
};

#endif // TOPWIDGET_H
