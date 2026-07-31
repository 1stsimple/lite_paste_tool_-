#include "TextNormalizer.h"

#include <QStringList>

namespace TextNormalizer
{

QString unifyLineEndings(const QString &text)
{
    if (text.isEmpty()) {
        return text;
    }
    QString result = text;
    result.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    result.replace(QChar('\r'), QChar('\n'));
    return result;
}

namespace
{

QString stripControlCharacters(const QString &text)
{
    QString result;
    result.reserve(text.size());
    for (const QChar &ch : text) {
        if (ch == QChar('\n') || ch == QChar('\t')) {
            result.append(ch);
            continue;
        }
        if (ch.category() == QChar::Other_Control) {
            continue;
        }
        result.append(ch);
    }
    return result;
}

QString trimTrailingWhitespacePerLine(const QString &text)
{
    const QStringList lines = text.split(QChar('\n'), Qt::KeepEmptyParts);
    QStringList trimmed;
    trimmed.reserve(lines.size());
    for (const QString &line : lines) {
        int end = line.size();
        while (end > 0 && (line[end - 1] == QChar(' ') || line[end - 1] == QChar('\t'))) {
            --end;
        }
        trimmed.append(line.left(end));
    }
    return trimmed.join(QChar('\n'));
}

QString collapseBlankLines(const QString &text)
{
    const QStringList lines = text.split(QChar('\n'), Qt::KeepEmptyParts);
    QStringList result;
    result.reserve(lines.size());
    bool previousBlank = false;
    for (const QString &line : lines) {
        const bool blank = line.isEmpty();
        if (blank && previousBlank) {
            continue;
        }
        result.append(line);
        previousBlank = blank;
    }
    return result.join(QChar('\n'));
}

} // namespace

QString normalize(const QString &text, const Options &options)
{
    if (text.isEmpty()) {
        return text;
    }

    QString result = unifyLineEndings(text);

    if (options.stripControlCharacters) {
        result = stripControlCharacters(result);
    }
    if (options.trimTrailingWhitespace) {
        result = trimTrailingWhitespacePerLine(result);
    }
    if (options.collapseBlankLines) {
        result = collapseBlankLines(result);
    }

    return result;
}

} // namespace TextNormalizer
