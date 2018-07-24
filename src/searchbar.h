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

private:
    QStringListModel m_completionModel;
    QCompleter m_completer;
    std::vector<std::string> m_urlList;
    QString m_currentHost;

private slots:
    void updateCompletion(const QString& text);
    void openCompletion(const QModelIndex& index);
    void openTitle();
};

#endif // SEARCHBAR_H
