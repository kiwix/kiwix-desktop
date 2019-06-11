#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>
#include <QPushButton>

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
private:
    QStringListModel m_completionModel;
    QCompleter m_completer;
    std::vector<std::string> m_urlList;
    QString m_currentZimId;
    SearchButton m_button;

private slots:
    void updateCompletion(const QString& text);
    void openCompletion(const QModelIndex& index);
};

#endif // SEARCHBAR_H
