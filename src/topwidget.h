#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QToolBar>
#include <QLineEdit>
#include <QWebEnginePage>

#include "searchbar.h"

class TopWidget : public QToolBar
{
    Q_OBJECT
public:
    explicit TopWidget(QWidget *parent = nullptr);
    virtual ~TopWidget();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    SearchBar m_searchEntry;
    QAction* mp_historyBackAction;
    QAction* mp_historyForwardAction;
    QPoint m_cursorPos;
    ulong m_timestamp;

private slots:
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
};

#endif // TOPWIDGET_H
