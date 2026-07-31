#include <QtTest>

#include "utils/TextNormalizer.h"

class TextNormalizerTest : public QObject
{
    Q_OBJECT

private slots:
    void emptyTextStaysEmpty();
    void unifiesAllLineEndingStyles();
    void stripsTrailingWhitespacePerLine();
    void collapsesConsecutiveBlankLines();
    void stripsControlCharactersButKeepsTabAndNewline();
    void optionsCanBeDisabledIndividually();
};

void TextNormalizerTest::emptyTextStaysEmpty()
{
    QCOMPARE(TextNormalizer::normalize(QString()), QString());
    QCOMPARE(TextNormalizer::unifyLineEndings(QString()), QString());
}

void TextNormalizerTest::unifiesAllLineEndingStyles()
{
    QCOMPARE(TextNormalizer::unifyLineEndings(QStringLiteral("a\r\nb\rc\nd")), QStringLiteral("a\nb\nc\nd"));
}

void TextNormalizerTest::stripsTrailingWhitespacePerLine()
{
    const QString input = QStringLiteral("line1   \nline2\t\t\nline3");
    QCOMPARE(TextNormalizer::normalize(input), QStringLiteral("line1\nline2\nline3"));
}

void TextNormalizerTest::collapsesConsecutiveBlankLines()
{
    const QString input = QStringLiteral("a\n\n\n\nb");
    QCOMPARE(TextNormalizer::normalize(input), QStringLiteral("a\n\nb"));
}

void TextNormalizerTest::stripsControlCharactersButKeepsTabAndNewline()
{
    const QString input = QStringLiteral("a\tb\nc") + QChar(0x0001) + QStringLiteral("d");
    QCOMPARE(TextNormalizer::normalize(input), QStringLiteral("a\tb\ncd"));
}

void TextNormalizerTest::optionsCanBeDisabledIndividually()
{
    TextNormalizer::Options options;
    options.trimTrailingWhitespace = false;
    options.collapseBlankLines = false;
    options.stripControlCharacters = false;

    const QString input = QStringLiteral("a   \n\n\nb");
    QCOMPARE(TextNormalizer::normalize(input, options), input);
}

QTEST_APPLESS_MAIN(TextNormalizerTest)
#include "test_text_normalizer.moc"
