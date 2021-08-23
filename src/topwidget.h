#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include <QToolBar>
#include <QLineEdit>
#include <QWebEnginePage>
#include <QScopedPointer>

#include "searchbar.h"

class TopWidget : public QToolBar
{
    Q_OBJECT
public:
    explicit TopWidget(QWidget *parent = nullptr);
    virtual ~TopWidget();

    SearchBar &getSearchBar() { return m_searchEntry; };

public slots:
    void handleWebActionEnabledChanged(QWebEnginePage::WebAction action, bool enabled);
    void updateBackForwardButtons();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    SearchBar m_searchEntry;
    QPoint m_cursorPos;
    ulong m_timestamp;
    QScopedPointer<QMenu, QScopedPointerDeleteLater> back_menu;
    QScopedPointer<QMenu, QScopedPointerDeleteLater> forward_menu;

    QToolButton* getBackButton() const;
    QToolButton* getForwardButton() const;
};

#endif // TOPWIDGET_H
