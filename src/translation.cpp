#include "translation.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <stdexcept>

Translation::Translation()
{
}

void Translation::setTranslation(QLocale locale)
{
    auto defaultText = JsonFileToQMap(":/i18n/en.json");
    if (defaultText.isEmpty()) {
        throw std::runtime_error("Invalid translation file");
    }
    if (locale.bcp47Name() == "en") {
        m_translations = defaultText;
        return;
    }
    m_translations = JsonFileToQMap(":/i18n/" + locale.bcp47Name() + ".json");
    for (auto &key : defaultText.keys()) {
        if (!m_translations.contains(key) || m_translations.value(key).isEmpty()) {
            m_translations.insert(key, defaultText.value(key));
        }
    }
}

QMap<QString, QString> Translation::JsonFileToQMap(const QString &filePath)
{
    QMap<QString, QString> translations;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return translations;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isNull()) {
        return translations;
    }
    auto jsonObj = doc.object();
    for (auto &key : jsonObj.keys()) {
        translations.insert(key, jsonObj.value(key).toString());
    }
    return translations;
}

QString Translation::getText(const QString &key)
{
    return (m_translations.contains(key)) ? m_translations.value(key) : key;
}
