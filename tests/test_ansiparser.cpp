/*
 * Unit tests for AnsiParser (NakumiTerm)
 *
 * Tests ANSI escape sequence parsing including SGR (colors),
 * cursor movement, and text segmentation.
 */

#include <QtTest/QtTest>
#include "ansiparser.h"

class TestAnsiParser : public QObject {
    Q_OBJECT

private slots:
    void testPlainText();
    void testNewlines();
    void testBoldAttribute();
    void testForegroundColors();
    void testBackgroundColors();
    void testResetAttributes();
    void testCursorMovement();
    void testMultipleSGRParams();
    void testBrightColors();
    void testTabExpansion();
    void testCarriageReturn();
    void testEmptyInput();
    void testPartialEscapeSequence();
};

void TestAnsiParser::testPlainText() {
    AnsiParser parser;
    auto lines = parser.parse("Hello, World!");
    QCOMPARE(lines.size(), 1);
    QCOMPARE(lines[0].segments.size(), 1);
    QCOMPARE(lines[0].segments[0].text, QStringLiteral("Hello, World!"));
}

void TestAnsiParser::testNewlines() {
    AnsiParser parser;
    auto lines = parser.parse("Line 1\nLine 2\nLine 3");
    QCOMPARE(lines.size(), 3);
    QCOMPARE(lines[0].segments[0].text, QStringLiteral("Line 1"));
    QCOMPARE(lines[1].segments[0].text, QStringLiteral("Line 2"));
    QCOMPARE(lines[2].segments[0].text, QStringLiteral("Line 3"));
}

void TestAnsiParser::testBoldAttribute() {
    AnsiParser parser;
    auto lines = parser.parse("\033[1mBold Text\033[0m");
    QCOMPARE(lines.size(), 1);
    /* After ESC[1m, the segment should be bold */
    bool foundBold = false;
    for (const auto &seg : lines[0].segments) {
        if (seg.text == QStringLiteral("Bold Text")) {
            QVERIFY(seg.bold);
            foundBold = true;
        }
    }
    QVERIFY(foundBold);
}

void TestAnsiParser::testForegroundColors() {
    AnsiParser parser;
    /* ESC[31m = Red foreground */
    auto lines = parser.parse("\033[31mRed Text\033[0m");
    QCOMPARE(lines.size(), 1);
    bool foundRed = false;
    for (const auto &seg : lines[0].segments) {
        if (seg.text == QStringLiteral("Red Text")) {
            QCOMPARE(seg.foreground, QColor(0xCC, 0x00, 0x00));
            foundRed = true;
        }
    }
    QVERIFY(foundRed);
}

void TestAnsiParser::testBackgroundColors() {
    AnsiParser parser;
    /* ESC[42m = Green background */
    auto lines = parser.parse("\033[42mGreen BG\033[0m");
    QCOMPARE(lines.size(), 1);
    bool foundGreen = false;
    for (const auto &seg : lines[0].segments) {
        if (seg.text == QStringLiteral("Green BG")) {
            QCOMPARE(seg.background, QColor(0x00, 0xCC, 0x00));
            foundGreen = true;
        }
    }
    QVERIFY(foundGreen);
}

void TestAnsiParser::testResetAttributes() {
    AnsiParser parser;
    auto lines = parser.parse("\033[1;31mBoldRed\033[0mNormal");
    QCOMPARE(lines.size(), 1);
    QVERIFY(lines[0].segments.size() >= 2);

    /* Last segment should have default attributes (reset) */
    const auto &lastSeg = lines[0].segments.last();
    QCOMPARE(lastSeg.text, QStringLiteral("Normal"));
    QVERIFY(!lastSeg.bold);
    QCOMPARE(lastSeg.foreground, QColor(0xE2, 0xE8, 0xF0));
}

void TestAnsiParser::testCursorMovement() {
    AnsiParser parser;
    /* ESC[5;10H = Move cursor to row 5, col 10 */
    parser.parse("\033[5;10H");
    QCOMPARE(parser.cursorRow(), 4); /* 0-indexed */
    QCOMPARE(parser.cursorCol(), 9);
}

void TestAnsiParser::testMultipleSGRParams() {
    AnsiParser parser;
    /* ESC[1;3;4m = Bold + Italic + Underline */
    auto lines = parser.parse("\033[1;3;4mStyled\033[0m");
    QCOMPARE(lines.size(), 1);
    bool foundStyled = false;
    for (const auto &seg : lines[0].segments) {
        if (seg.text == QStringLiteral("Styled")) {
            QVERIFY(seg.bold);
            QVERIFY(seg.italic);
            QVERIFY(seg.underline);
            foundStyled = true;
        }
    }
    QVERIFY(foundStyled);
}

void TestAnsiParser::testBrightColors() {
    AnsiParser parser;
    /* ESC[91m = Bright Red foreground */
    auto lines = parser.parse("\033[91mBright Red\033[0m");
    QCOMPARE(lines.size(), 1);
    bool found = false;
    for (const auto &seg : lines[0].segments) {
        if (seg.text == QStringLiteral("Bright Red")) {
            QCOMPARE(seg.foreground, QColor(0xFF, 0x55, 0x55));
            found = true;
        }
    }
    QVERIFY(found);
}

void TestAnsiParser::testTabExpansion() {
    AnsiParser parser;
    auto lines = parser.parse("A\tB");
    QCOMPARE(lines.size(), 1);
    /* Tab should expand to spaces */
    QString fullText;
    for (const auto &seg : lines[0].segments) {
        fullText += seg.text;
    }
    QVERIFY(fullText.contains(QLatin1Char('A')));
    QVERIFY(fullText.contains(QLatin1Char('B')));
    /* Tab expands to align to 8-column boundary */
    QVERIFY(fullText.length() > 2);
}

void TestAnsiParser::testCarriageReturn() {
    AnsiParser parser;
    parser.parse("Hello\r");
    QCOMPARE(parser.cursorCol(), 0);
}

void TestAnsiParser::testEmptyInput() {
    AnsiParser parser;
    auto lines = parser.parse("");
    QCOMPARE(lines.size(), 0);
}

void TestAnsiParser::testPartialEscapeSequence() {
    AnsiParser parser;
    /* An incomplete escape sequence shouldn't crash */
    auto lines = parser.parse("Hello\033");
    QVERIFY(lines.size() >= 1);
    QCOMPARE(lines[0].segments[0].text, QStringLiteral("Hello"));
}

QTEST_MAIN(TestAnsiParser)
#include "test_ansiparser.moc"
