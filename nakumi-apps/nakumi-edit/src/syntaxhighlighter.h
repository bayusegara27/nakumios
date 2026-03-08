/*
 * SyntaxHighlighter - Multi-language syntax highlighting
 *
 * Supports C++, Bash, and plain text.
 */

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    enum Language {
        PlainText,
        Cpp,
        Bash,
    };

    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

    void setLanguage(Language lang);
    Language language() const { return m_language; }

    static Language detectLanguage(const QString &filePath);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    void setupCppRules();
    void setupBashRules();

    Language m_language = PlainText;
    QVector<HighlightRule> m_rules;

    /* Multi-line comment state */
    QRegularExpression m_commentStartExpr;
    QRegularExpression m_commentEndExpr;
    QTextCharFormat m_commentFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
