#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QIcon>

class SearchBar : public QLineEdit
{
    Q_OBJECT
public:
    SearchBar(QWidget *parent = nullptr);

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    QStringListModel m_completionModel;
    QCompleter m_completer;
    std::vector<std::string> m_urlList;
    QString m_currentHost;
    QIcon m_icon;

private slots:
    void updateCompletion(const QString& text);
    void openCompletion(const QModelIndex& index);
    void openTitle();
};

#endif // SEARCHBAR_H
