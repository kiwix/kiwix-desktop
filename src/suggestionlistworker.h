#ifndef SUGGESTIONLISTWORKER_H
#define SUGGESTIONLISTWORKER_H

#include <QUrl>
#include <QVector>
#include <QThread>

class SuggestionListWorker : public QThread
{
    Q_OBJECT
public:
    static int getFetchSize() { return 15; };

    SuggestionListWorker(const QString& text, int token, int start, QObject *parent = nullptr);
    void run() override;

signals:
    void searchFinished(const QStringList& suggestions, const QVector<QUrl>& urlList, int token);

private:
    QString m_text;
    int m_token = 0;
    int m_start;
};

#endif // SUGGESTIONLISTWORKER_H
