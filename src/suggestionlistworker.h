#ifndef SUGGESTIONLISTWORKER_H
#define SUGGESTIONLISTWORKER_H

#include <QThread>

struct SuggestionData;

class SuggestionListWorker : public QThread
{
    Q_OBJECT
public:
    SuggestionListWorker(const QString& text, int token, QObject *parent = nullptr);
    void run() override;

signals:
    void searchFinished(const QList<SuggestionData>& suggestionList, int token);

private:
    QString m_text;
    int m_token = 0;
};

#endif // SUGGESTIONLISTWORKER_H
