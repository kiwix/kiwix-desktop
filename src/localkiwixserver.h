#ifndef LOCALKIWIXSERVER_H
#define LOCALKIWIXSERVER_H

#include <QDialog>
#include <kiwix/kiwixserve.h>

namespace Ui {
class LocalKiwixServer;
}

class LocalKiwixServer : public QDialog
{
    Q_OBJECT

public:
    explicit LocalKiwixServer(QWidget *parent = nullptr);
    ~LocalKiwixServer();

public slots:
    void runOrStopServer();
    void openInBrowser();

private:
    Ui::LocalKiwixServer *ui;
    kiwix::KiwixServe* mp_server;
    bool m_active = false;
    QString m_ipAddress;
    int m_port;
};

#endif // LOCALKIWIXSERVER_H
