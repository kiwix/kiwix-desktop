#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QString>
#include <QMap>
#include <QLocale>

class Translation
{
public:
    Translation();

    void setTranslation(QLocale locale);
    QString getText(const QString &key);

private:
    QMap<QString, QString> JsonFileToQMap(const QString &filePath);

private:
    QMap<QString, QString> m_translations;
};

#endif // TRANSLATION_H
