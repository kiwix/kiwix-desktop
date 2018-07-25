#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMenu>

class MainMenu : public QMenu
{
public:
    MainMenu(QWidget *parent=nullptr);

private:
    QMenu m_fileMenu;
    QMenu m_editMenu;
    QMenu m_viewMenu;
    QMenu m_toolsMenu;
    QMenu m_helpMenu;
};

#endif // MAINMENU_H
