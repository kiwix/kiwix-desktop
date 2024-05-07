#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>
#include <QPushButton>
#include <QUrl>
#include <QTimer>
#include <QThread>
#include <QToolBar>

class SearchButton : public QPushButton {
    Q_OBJECT
public:
    SearchButton(QWidget *parent = nullptr);

public slots:
    void set_searchMode(bool searchMode);
    void on_buttonClicked();

protected:
    bool m_searchMode;
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
    QStringListModel m_completionModel;
    QCompleter m_completer;
    QVector<QUrl> m_urlList;
    SearchButton m_button;
    QString m_title;
    QString m_searchbarInput;
    bool m_returnPressed = false;
    QTimer* mp_typingTimer;
    int m_token;

private slots:
    void updateCompletion();
    void openCompletion(const QModelIndex& index);
    void openCompletion(const QString& text, int index);
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
};
#endif // SEARCHBAR_H
