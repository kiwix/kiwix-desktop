#ifndef SUGGESTIONLISTWORKER_H
#define SUGGESTIONLISTWORKER_H

#include <QThread>

struct SuggestionData;

class SuggestionListWorker : public QThread
{
    Q_OBJECT
public:
    static int getFetchSize() { return 15; };

    SuggestionListWorker(const QString& text, int token, int start, QObject *parent = nullptr);
    void run() override;

signals:
    void searchFinished(const QList<SuggestionData>& suggestionList, int token);

private:
    QString m_text;
    int m_token = 0;
    int m_start;
};

#endif // SUGGESTIONLISTWORKER_H
