#include "settingsmanager.h"
#include "kiwixapp.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <kiwix/tools.h>
#include <QLocale>
#include <QList>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
    m_settings("Kiwix", "Kiwix-desktop"),
    m_view(nullptr)
{
    initSettings();
}

SettingsView* SettingsManager::getView()
{
    if (m_view == nullptr) {
        auto view = new SettingsView();
        view->init(m_zoomFactor * 100, m_downloadDir, m_monitorDir, m_appLangIndex, m_moveToTrash, m_reopenTab);
        connect(view, &QObject::destroyed, this, [=]() { m_view = nullptr; });
        m_view = view;
    }
    return m_view;
}

void SettingsManager::setSettings(const QString &key, const QVariant &value)
{
    m_settings.setValue(key, value);
}

void SettingsManager::deleteSettings(const QString &key)
{
    m_settings.remove(key);
}

bool SettingsManager::settingsExists(const QString &key)
{
    return m_settings.contains(key);
}

QVariant SettingsManager::getSettings(const QString &key)
{
    return m_settings.value(key);
}

qreal SettingsManager::getZoomFactorByZimId(const QString &id)
{
    auto zoomFactor = m_zoomFactor;
    QString key = id + "/zoomFactor";
    if (settingsExists(key)) {
        zoomFactor = getSettings(key).toDouble();
    }
    return zoomFactor;
}

void SettingsManager::setKiwixServerPort(int port)
{
    m_kiwixServerPort = port;
    m_settings.setValue("localKiwixServer/port", port);
    emit(portChanged(port));
}

void SettingsManager::setKiwixServerIpAddress(QString ipAddress)
{
    m_kiwixServerIpAddress = ipAddress;
    m_settings.setValue("localKiwixServer/ipAddress", ipAddress);
}

void SettingsManager::setZoomFactor(qreal zoomFactor)
{
    m_zoomFactor = zoomFactor;
    m_settings.setValue("view/zoomFactor", zoomFactor);
    emit(zoomChanged(zoomFactor));
}

void SettingsManager::setDownloadDir(QString downloadDir)
{
    downloadDir = QDir::toNativeSeparators(downloadDir);
    m_downloadDir = downloadDir;
    m_settings.setValue("download/dir", downloadDir);
    emit(downloadDirChanged(downloadDir));
}

void SettingsManager::setMonitorDir(QString monitorDir)
{
    monitorDir = QDir::toNativeSeparators(monitorDir);
    m_monitorDir = monitorDir;
    m_settings.setValue("monitor/dir", monitorDir);
    emit(monitorDirChanged(monitorDir));
}

void SettingsManager::setMoveToTrash(bool moveToTrash)
{
    m_moveToTrash = moveToTrash;
    setSettings("moveToTrash", m_moveToTrash);
    emit(moveToTrashChanged(m_moveToTrash));
}

void SettingsManager::setReopenTab(bool reopenTab)
{
    m_reopenTab = reopenTab;
    setSettings("reopenTab", m_reopenTab);
    emit(reopenTabChanged(reopenTab));
}

QList<QVariant> SettingsManager::flattenPair(FilterList pairList)
{
    QList<QVariant> res;
    for (auto &pair : pairList) {
        res.push_back(pair.first+"|"+pair.second);
    }
    return res;
}

SettingsManager::FilterList SettingsManager::deducePair(QList<QVariant> variantList)
{
    FilterList pairList;
    for (auto &variant : variantList) {
        QString str = variant.toString();
        auto pairs = str.split('|');
        pairList.push_back({pairs[0], pairs[1]});
    }
    return pairList;
}

void SettingsManager::setLanguage(FilterList langList)  // This regards the search filter languages
{
    m_langList = flattenPair(langList);
    setSettings("language", m_langList);
    emit(languageChanged(m_langList));
}

void SettingsManager::setAppLanguage(int languageIndex) // This regards the application language
{   
    m_appLangIndex = languageIndex;
    KiwixApp::instance()->setAppLanguage();
    m_settings.setValue("app/languageIndex", languageIndex);
}

void SettingsManager::setCategory(FilterList categoryList)
{
    m_categoryList = flattenPair(categoryList);
    setSettings("category", m_categoryList);
    emit(categoryChanged(m_categoryList));
}

void SettingsManager::setContentType(FilterList contentTypeList)
{
    m_contentTypeList = flattenPair(contentTypeList);
    setSettings("contentType", m_contentTypeList);
    emit(contentTypeChanged(m_contentTypeList));
}

QString SettingsManager::getLanguageName(QString fileName) // Get native language name based on filename
{
    QString dirPath = "resources/i18n";
    QFile file(dirPath + "/" + fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Error parsing JSON " << fileName << ":" << error.errorString();
        return QString();
    }

    QJsonObject obj = doc.object();
    if (obj.contains("name")) { return obj.value("name").toString(); }
    else { qDebug() << "Cannot find element \"name\" in " << fileName; }

    return QString();
}

void SettingsManager::loadAvailableLanguages() {
    QString dirPath = "resources/i18n";
    QDir directory(dirPath);
    if (!directory.exists()) {
        qDebug() << "Directory does not exist:" << dirPath;
        return;
    }
    QStringList jsonFiles = directory.entryList(QDir::Files);
    for(QString &fileName : jsonFiles) {
        if(getLanguageName(fileName).isEmpty() || fileName == "qqq.json") { continue; }
        m_appLangCodes.append(fileName.split(".")[0]);
    }
}

void SettingsManager::initSettings()
{
    m_kiwixServerPort = m_settings.value("localKiwixServer/port", 8080).toInt();
    m_zoomFactor = m_settings.value("view/zoomFactor", 1).toDouble();
    m_downloadDir = m_settings.value("download/dir", QString::fromStdString(kiwix::getDataDirectory())).toString();
    m_kiwixServerIpAddress = m_settings.value("localKiwixServer/ipAddress", QString("0.0.0.0")).toString();
    m_monitorDir = m_settings.value("monitor/dir", QString("")).toString();
    m_moveToTrash = m_settings.value("moveToTrash", true).toBool();
    m_reopenTab = m_settings.value("reopenTab", false).toBool();
    // This regards the application language
    loadAvailableLanguages();

    int i = 0, defaultLanguage = -1;
    for (const QString& code : m_appLangCodes) {
        QString name = QLocale::languageToString(QLocale(code).language());
        if(name == "English") 
        { 
            defaultLanguage = i; 
            break;
        }
        i++;
    }
    m_appLangIndex = m_settings.value("app/languageIndex", defaultLanguage).toInt();
    // Below regards the search filters
    QString defaultLang = QLocale::languageToString(QLocale().language()) + '|' + QLocale().name().split("_").at(0); // TODO maybe needs fix
    QList<QString> defaultLangList; // Qt5 QList doesn't support supplying a constructor list
    defaultLangList.append(defaultLang);
    QVariant defaultLangVariant(defaultLangList);
    m_langList = m_settings.value("language", defaultLangVariant).toList();
    /*
     * Qt5 & Qt6 have slightly different behaviors with regards to initializing QVariant.
     * The below approach is specifically chosen to work with both versions.
     * m_langList initialized with defaultLang should be of the form:
     *
     * (QVariant(QString, "English|en"))
     *
     * and not
     *
     * QList(QVariant(QChar, 'E'), QVariant(QChar, 'n'), QVariant(QChar, 'g'), ...
     */
    m_categoryList = m_settings.value("category", {}).toList();
    m_contentTypeList = m_settings.value("contentType", {}).toList();
}
