#include "stylesheettemplate.h"

#include <QRegularExpression>
#include <QDebug>

namespace
{

QString keepThemeBlocks(const QString &styleSheet, const QString &themeName)
{
    static const QRegularExpression blockStart("@theme\\s+([\\w-]+)\\s*\\{");
    QString result;
    qsizetype pos = 0;
    QRegularExpressionMatch match;
    while ((match = blockStart.match(styleSheet, pos)).hasMatch()) {
        result += styleSheet.mid(pos, match.capturedStart() - pos);
        qsizetype end = match.capturedEnd();
        for (int depth = 1; depth > 0 && end < styleSheet.size(); ++end) {
            if (styleSheet[end] == '{')
                ++depth;
            else if (styleSheet[end] == '}')
                --depth;
        }
        if (match.captured(1) == themeName)
            result += styleSheet.mid(match.capturedEnd(), end - 1 - match.capturedEnd());
        pos = end;
    }
    return result + styleSheet.mid(pos);
}

}

QString renderStyleSheetTemplate(QString styleSheet, const QString &themeName,
                                 const QJsonObject &values)
{
    styleSheet = keepThemeBlocks(styleSheet, themeName);

    for (auto it = values.begin(); it != values.end(); ++it) {
        if (it.value().isNull()) {
            const QRegularExpression declaration(
                "[^;{}]*\\{\\{" + QRegularExpression::escape(it.key()) + "\\}\\}[^;{}]*;?");
            styleSheet.remove(declaration);
        }
    }
    static const QRegularExpression emptyRule("[^{}]*\\{\\s*\\}");
    styleSheet.remove(emptyRule);

    for (auto it = values.begin(); it != values.end(); ++it) {
        if (it.value().isString())
            styleSheet.replace("{{" + it.key() + "}}", it.value().toString());
    }

    static const QRegularExpression unknownName("\\{\\{\\w+\\}\\}");
    if (styleSheet.contains(unknownName))
        qWarning() << "Unknown theme value in style sheet:"
                   << unknownName.match(styleSheet).captured();
    return styleSheet;
}
