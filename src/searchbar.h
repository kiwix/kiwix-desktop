#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>
#include <QUrl>
#include <QTimer>
#include <QThread>


class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    SearchBar(QWidget *parent = nullptr);
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
    QAction *m_action;
    QString m_title;
    QString m_searchbarInput;
    bool m_returnPressed = false;
    QTimer* mp_typingTimer;
    int m_token;
    void set_searchMode(bool searchMode);
    bool m_searchMode;

private slots:
    void updateCompletion();
    void openCompletion(const QModelIndex& index);
    void openCompletion(const QString& text, int index);
    void on_action_triggered();
};

#endif // SEARCHBAR_H
