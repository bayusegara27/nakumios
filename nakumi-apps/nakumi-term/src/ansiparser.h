/*
 * AnsiParser - ANSI escape code parser for terminal emulation
 *
 * Parses VT100/VT220/xterm ANSI escape sequences and extracts
 * text segments with associated formatting attributes.
 */

#ifndef ANSIPARSER_H
#define ANSIPARSER_H

#include <QObject>
#include <QColor>
#include <QString>
#include <QVector>

struct TextSegment {
    QString text;
    QColor foreground = QColor(0xE2, 0xE8, 0xF0); /* #E2E8F0 */
    QColor background = Qt::transparent;
    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool inverse = false;
};

struct TerminalLine {
    QVector<TextSegment> segments;
};

class AnsiParser : public QObject {
    Q_OBJECT

public:
    explicit AnsiParser(QObject *parent = nullptr);

    /* Parse raw bytes and return structured lines */
    QVector<TerminalLine> parse(const QByteArray &data);

    /* Reset parser state */
    void reset();

    /* Get current cursor position */
    int cursorRow() const { return m_cursorRow; }
    int cursorCol() const { return m_cursorCol; }

private:
    enum State {
        Normal,
        Escape,      /* ESC received */
        CSI,         /* ESC [ received (Control Sequence Introducer) */
        OSC,         /* ESC ] received (Operating System Command) */
    };

    void processChar(char ch);
    void processCSI(char finalByte);
    void applySGR(int code);
    void resetAttributes();

    /* Standard 8-color + bright 8-color palette */
    static QColor colorFromIndex(int index, bool bright);

    State m_state = Normal;
    QString m_paramBuffer;
    QVector<TerminalLine> m_lines;
    TerminalLine m_currentLine;
    TextSegment m_currentSegment;

    int m_cursorRow = 0;
    int m_cursorCol = 0;

    /* Current text attributes */
    QColor m_fg{0xE2, 0xE8, 0xF0};
    QColor m_bg = Qt::transparent;
    bool m_bold = false;
    bool m_italic = false;
    bool m_underline = false;
    bool m_inverse = false;
};

#endif // ANSIPARSER_H
