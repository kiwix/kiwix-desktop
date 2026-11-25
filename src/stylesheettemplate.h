#ifndef STYLESHEETTEMPLATE_H
#define STYLESHEETTEMPLATE_H

#include <QJsonObject>
#include <QString>

/**
 * @brief Renders a style sheet template for one theme.
 *
 * Templates are ordinary Qt style sheets with two additions:
 * - `{{NAME}}` is replaced by the theme's value for NAME (see resources/themes).
 *   A null value means the theme doesn't set it: every declaration using
 *   NAME is dropped. Rules left without declarations are dropped too.
 * - `@theme NAME { ... }` keeps its content only for the theme NAME.
 *
 * @param styleSheet  The template.
 * @param themeName   The theme to render, e.g. "light" or "dark".
 * @param values      The theme's values, by name.
 * @return The style sheet for that theme.
 */
QString renderStyleSheetTemplate(QString styleSheet, const QString &themeName,
                                 const QJsonObject &values);

#endif // STYLESHEETTEMPLATE_H
