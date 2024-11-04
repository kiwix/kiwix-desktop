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
#include "multizimbutton.h"

class QTreeView;

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

    typedef void (SearchBarLineEdit::*NewSuggestionHandlerFuncPtr)(int start);

public:
    SearchBarLineEdit(QWidget *parent = nullptr);
    void hideSuggestions();
    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void on_currentTitleChanged(const QString &title);
    void clearSuggestions();

protected:
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
private:
    SuggestionListModel m_suggestionModel;
    QTreeView *m_suggestionView;
    QCompleter m_completer;
    QString m_title;
    QString m_searchbarInput;
    QString m_suggestionsAreValidFor;
    bool m_returnPressed = false;
    QTimer* mp_typingTimer;
    int m_token;
    bool m_moreSuggestionsAreAvailable = false;

    /* We only fetch more suggestions when the user is at the end and tries to
       scroll again. This variable is set whenever the user scrolled to the end,
       indicating the next scroll should trigger a fetch more action. */
    bool m_aboutToScrollPastEnd = false;

private slots:
    void updateCompletion();
    void fetchMoreSuggestions();
    void onScroll(int value);
    void openCompletion(const QModelIndex& index);
    void onInitialSuggestions(int);
    void onAdditionalSuggestions(int start);
    void fetchSuggestions(NewSuggestionHandlerFuncPtr callback);

    QModelIndex getDefaulSuggestionIndex() const;
    QRect getCompleterRect() const;
};


class SearchBar : public QToolBar {
    Q_OBJECT
public:
    SearchBar(QWidget *parent = nullptr);
    SearchBarLineEdit& getLineEdit() { return m_searchBarLineEdit; };
    MultiZimButton& getMultiZimButton() { return m_multiZimButton; };

signals:
    void currentTitleChanged(const QString &title);

private:
    SearchBarLineEdit m_searchBarLineEdit;
    BookmarkButton m_bookmarkButton;
    MultiZimButton m_multiZimButton;
};
#endif // SEARCHBAR_H
