#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>
#include <QToolButton>
#include <QUrl>
#include <QTimer>
#include <QThread>
#include <QToolBar>
#include "suggestionlistmodel.h"

class BookmarkButton : public QToolButton {
    Q_OBJECT
public:
    BookmarkButton(QWidget *parent = nullptr);

public slots:
    void update_display();
    void on_buttonClicked();
};

class SearchBarLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SearchBarLineEdit(QWidget *parent = nullptr);
    void hideSuggestions();

public slots:
    void on_currentTitleChanged(const QString &title);
    void clearSuggestions();

protected:
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
private:
    SuggestionListModel m_suggestionModel;
    QCompleter m_completer;
    QString m_title;
    QString m_searchbarInput;
    bool m_returnPressed = false;
    QTimer* mp_typingTimer;
    int m_token;

private slots:
    void updateCompletion();
    void openCompletion(const QModelIndex& index);

    QModelIndex getDefaulSuggestionIndex() const;
};


class SearchBar : public QToolBar {
    Q_OBJECT
public:
    SearchBar(QWidget *parent = nullptr);
    SearchBarLineEdit& getLineEdit() { return m_searchBarLineEdit; };

signals:
    void currentTitleChanged(const QString &title);

private:
    SearchBarLineEdit m_searchBarLineEdit;
    BookmarkButton m_bookmarkButton;
};
#endif // SEARCHBAR_H
