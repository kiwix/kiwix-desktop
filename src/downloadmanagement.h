#ifndef DOWNLOADMANAGEMENT_H
#define DOWNLOADMANAGEMENT_H

#include <QMap>
#include <QString>
#include <QVariant>

typedef QMap<QString, QVariant> DownloadInfo;

class DownloadState
{
public:
    double progress = 0;
    QString completedLength;
    QString downloadSpeed;
    bool paused = false;

public:
    void update(const DownloadInfo& info);
};

#endif // DOWNLOADMANAGEMENT_H
