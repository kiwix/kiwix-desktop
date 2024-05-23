#include "downloadmanagement.h"

////////////////////////////////////////////////////////////////////////////////
// DowloadState
////////////////////////////////////////////////////////////////////////////////

namespace
{

QString convertToUnits(double bytes)
{
    QStringList units = {"bytes", "KB", "MB", "GB", "TB", "PB", "EB"};
    int unitIndex = 0;
    while (bytes >= 1024 && unitIndex < units.size()) {
        bytes /= 1024;
        unitIndex++;
    }

    const auto preciseBytes = QString::number(bytes, 'g', 3);
    return preciseBytes + " " + units[unitIndex];
}

} // unnamed namespace

void DownloadState::update(const DownloadInfo& downloadInfos)
{
    double percent = downloadInfos["completedLength"].toDouble() / downloadInfos["totalLength"].toDouble();
    percent *= 100;
    percent = QString::number(percent, 'g', 3).toDouble();
    auto completedLength = convertToUnits(downloadInfos["completedLength"].toDouble());
    auto downloadSpeed = convertToUnits(downloadInfos["downloadSpeed"].toDouble()) + "/s";
    const bool paused = downloadInfos["status"] == "paused";
    *this = {percent, completedLength, downloadSpeed, paused};
}
