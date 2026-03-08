/*
 * SyntaxHighlighter implementation
 */

#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
}

SyntaxHighlighter::Language SyntaxHighlighter::detectLanguage(
    const QString &filePath) {
    if (filePath.endsWith(QLatin1String(".cpp")) ||
        filePath.endsWith(QLatin1String(".cxx")) ||
        filePath.endsWith(QLatin1String(".cc")) ||
        filePath.endsWith(QLatin1String(".c")) ||
        filePath.endsWith(QLatin1String(".h")) ||
        filePath.endsWith(QLatin1String(".hpp"))) {
        return Cpp;
    }
    if (filePath.endsWith(QLatin1String(".sh")) ||
        filePath.endsWith(QLatin1String(".bash")) ||
        filePath.endsWith(QLatin1String(".zsh"))) {
        return Bash;
    }
    return PlainText;
}

void SyntaxHighlighter::setLanguage(Language lang) {
    m_language = lang;
    m_rules.clear();

    switch (lang) {
    case Cpp:
        setupCppRules();
        break;
    case Bash:
        setupBashRules();
        break;
    default:
        break;
    }

    rehighlight();
}

void SyntaxHighlighter::setupCppRules() {
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor(0x6C, 0x5C, 0xE7)); /* Accent purple */
    keywordFmt.setFontWeight(QFont::Bold);

    const QStringList cppKeywords = {
        QStringLiteral("\\balignas\\b"), QStringLiteral("\\balignof\\b"),
        QStringLiteral("\\bauto\\b"), QStringLiteral("\\bbool\\b"),
        QStringLiteral("\\bbreak\\b"), QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\bcatch\\b"), QStringLiteral("\\bchar\\b"),
        QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bconstexpr\\b"), QStringLiteral("\\bcontinue\\b"),
        QStringLiteral("\\bdefault\\b"), QStringLiteral("\\bdelete\\b"),
        QStringLiteral("\\bdo\\b"), QStringLiteral("\\bdouble\\b"),
        QStringLiteral("\\belse\\b"), QStringLiteral("\\benum\\b"),
        QStringLiteral("\\bexplicit\\b"), QStringLiteral("\\bextern\\b"),
        QStringLiteral("\\bfalse\\b"), QStringLiteral("\\bfloat\\b"),
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\bfriend\\b"),
        QStringLiteral("\\bgoto\\b"), QStringLiteral("\\bif\\b"),
        QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
        QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"),
        QStringLiteral("\\bnew\\b"), QStringLiteral("\\bnoexcept\\b"),
        QStringLiteral("\\bnullptr\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"),
        QStringLiteral("\\bpublic\\b"), QStringLiteral("\\breturn\\b"),
        QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsigned\\b"),
        QStringLiteral("\\bsizeof\\b"), QStringLiteral("\\bstatic\\b"),
        QStringLiteral("\\bstatic_cast\\b"), QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\bswitch\\b"), QStringLiteral("\\btemplate\\b"),
        QStringLiteral("\\bthis\\b"), QStringLiteral("\\bthrow\\b"),
        QStringLiteral("\\btrue\\b"), QStringLiteral("\\btry\\b"),
        QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
        QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"),
        QStringLiteral("\\busing\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"),
        QStringLiteral("\\bwhile\\b"),
    };

    for (const auto &kw : cppKeywords) {
        m_rules.append({QRegularExpression(kw), keywordFmt});
    }

    /* Preprocessor directives */
    QTextCharFormat preprocFmt;
    preprocFmt.setForeground(QColor(0xCC, 0x99, 0x00));
    m_rules.append({QRegularExpression(QStringLiteral("^\\s*#[^\n]*")), preprocFmt});

    /* String literals */
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor(0x00, 0xCC, 0x66));
    m_rules.append({QRegularExpression(QStringLiteral("\"[^\"]*\"")), stringFmt});
    m_rules.append({QRegularExpression(QStringLiteral("'[^']*'")), stringFmt});

    /* Numbers */
    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor(0xFF, 0x99, 0x33));
    m_rules.append({QRegularExpression(QStringLiteral("\\b[0-9]+(\\.[0-9]+)?\\b")), numberFmt});

    /* Single-line comments */
    m_commentFormat.setForeground(QColor(0x60, 0x60, 0x80));
    m_commentFormat.setFontItalic(true);
    m_rules.append({QRegularExpression(QStringLiteral("//[^\n]*")), m_commentFormat});

    /* Multi-line comments */
    m_commentStartExpr = QRegularExpression(QStringLiteral("/\\*"));
    m_commentEndExpr = QRegularExpression(QStringLiteral("\\*/"));
}

void SyntaxHighlighter::setupBashRules() {
    /* Keywords */
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor(0x6C, 0x5C, 0xE7));
    keywordFmt.setFontWeight(QFont::Bold);

    const QStringList bashKeywords = {
        QStringLiteral("\\bif\\b"), QStringLiteral("\\bthen\\b"),
        QStringLiteral("\\belse\\b"), QStringLiteral("\\belif\\b"),
        QStringLiteral("\\bfi\\b"), QStringLiteral("\\bfor\\b"),
        QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bdo\\b"),
        QStringLiteral("\\bdone\\b"), QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\besac\\b"), QStringLiteral("\\bfunction\\b"),
        QStringLiteral("\\breturn\\b"), QStringLiteral("\\blocal\\b"),
        QStringLiteral("\\bexport\\b"), QStringLiteral("\\bsource\\b"),
        QStringLiteral("\\becho\\b"), QStringLiteral("\\bread\\b"),
        QStringLiteral("\\bset\\b"), QStringLiteral("\\bunset\\b"),
        QStringLiteral("\\bin\\b"),
    };

    for (const auto &kw : bashKeywords) {
        m_rules.append({QRegularExpression(kw), keywordFmt});
    }

    /* Variables */
    QTextCharFormat varFmt;
    varFmt.setForeground(QColor(0xFF, 0x55, 0x55));
    m_rules.append({QRegularExpression(QStringLiteral("\\$[A-Za-z_][A-Za-z0-9_]*")), varFmt});
    m_rules.append({QRegularExpression(QStringLiteral("\\$\\{[^}]+\\}")), varFmt});

    /* Strings */
    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor(0x00, 0xCC, 0x66));
    m_rules.append({QRegularExpression(QStringLiteral("\"[^\"]*\"")), stringFmt});
    m_rules.append({QRegularExpression(QStringLiteral("'[^']*'")), stringFmt});

    /* Comments */
    m_commentFormat.setForeground(QColor(0x60, 0x60, 0x80));
    m_commentFormat.setFontItalic(true);
    m_rules.append({QRegularExpression(QStringLiteral("#[^\n]*")), m_commentFormat});
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    /* Apply single-line rules */
    for (const auto &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    /* Multi-line comment handling (C++ only) */
    if (m_language != Cpp) return;

    setCurrentBlockState(0);
    int startIndex = 0;

    if (previousBlockState() != 1) {
        QRegularExpressionMatch match = m_commentStartExpr.match(text);
        startIndex = match.hasMatch() ? match.capturedStart() : -1;
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch =
            m_commentEndExpr.match(text, startIndex + 2);
        int endIndex = endMatch.hasMatch() ? endMatch.capturedStart() : -1;
        int commentLength;

        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, m_commentFormat);
        QRegularExpressionMatch nextMatch =
            m_commentStartExpr.match(text, startIndex + commentLength);
        startIndex = nextMatch.hasMatch() ? nextMatch.capturedStart() : -1;
    }
}
