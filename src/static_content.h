
#ifndef STATICCONTENT_H
#define STATICCONTENT_H

#include <QLocale>
#include <QString>
#include <utility>
#include <vector>

extern std::vector<QLocale::Language> S_LANGUAGES;
extern std::vector<std::pair<QString, QString>> S_CATEGORIES;

void initStaticContent();

#endif //STATICCONTENT_H
