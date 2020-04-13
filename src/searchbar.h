#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>
#include <QPushButton>
#include <QUrl>

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

class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    SearchBar(QWidget *parent = nullptr);

public slots:
    void on_currentTitleChanged(const QString &title);
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

private slots:
    void updateCompletion(const QString& text);
    void openCompletion(const QModelIndex& index);
};

#endif // SEARCHBAR_H
