/*
 * AnsiParser implementation
 */

#include "ansiparser.h"

AnsiParser::AnsiParser(QObject *parent)
    : QObject(parent) {
    resetAttributes();
}

void AnsiParser::reset() {
    m_state = Normal;
    m_paramBuffer.clear();
    m_lines.clear();
    m_currentLine = {};
    m_currentSegment = {};
    m_cursorRow = 0;
    m_cursorCol = 0;
    resetAttributes();
}

void AnsiParser::resetAttributes() {
    m_fg = QColor(0xE2, 0xE8, 0xF0);
    m_bg = Qt::transparent;
    m_bold = false;
    m_italic = false;
    m_underline = false;
    m_inverse = false;
}

QColor AnsiParser::colorFromIndex(int index, bool bright) {
    /* Standard 8 ANSI colors */
    static const QColor normal[] = {
        QColor(0x00, 0x00, 0x00), /* Black */
        QColor(0xCC, 0x00, 0x00), /* Red */
        QColor(0x00, 0xCC, 0x00), /* Green */
        QColor(0xCC, 0xCC, 0x00), /* Yellow */
        QColor(0x00, 0x00, 0xCC), /* Blue */
        QColor(0xCC, 0x00, 0xCC), /* Magenta */
        QColor(0x00, 0xCC, 0xCC), /* Cyan */
        QColor(0xCC, 0xCC, 0xCC), /* White */
    };
    static const QColor brightColors[] = {
        QColor(0x55, 0x55, 0x55), /* Bright Black */
        QColor(0xFF, 0x55, 0x55), /* Bright Red */
        QColor(0x55, 0xFF, 0x55), /* Bright Green */
        QColor(0xFF, 0xFF, 0x55), /* Bright Yellow */
        QColor(0x55, 0x55, 0xFF), /* Bright Blue */
        QColor(0xFF, 0x55, 0xFF), /* Bright Magenta */
        QColor(0x55, 0xFF, 0xFF), /* Bright Cyan */
        QColor(0xFF, 0xFF, 0xFF), /* Bright White */
    };

    if (index < 0 || index > 7) return QColor(0xE2, 0xE8, 0xF0);
    return bright ? brightColors[index] : normal[index];
}

void AnsiParser::applySGR(int code) {
    switch (code) {
    case 0: /* Reset */
        resetAttributes();
        break;
    case 1: m_bold = true; break;
    case 3: m_italic = true; break;
    case 4: m_underline = true; break;
    case 7: m_inverse = true; break;
    case 22: m_bold = false; break;
    case 23: m_italic = false; break;
    case 24: m_underline = false; break;
    case 27: m_inverse = false; break;
    /* Foreground colors 30-37 */
    case 30: case 31: case 32: case 33:
    case 34: case 35: case 36: case 37:
        m_fg = colorFromIndex(code - 30, m_bold);
        break;
    case 39: /* Default foreground */
        m_fg = QColor(0xE2, 0xE8, 0xF0);
        break;
    /* Background colors 40-47 */
    case 40: case 41: case 42: case 43:
    case 44: case 45: case 46: case 47:
        m_bg = colorFromIndex(code - 40, false);
        break;
    case 49: /* Default background */
        m_bg = Qt::transparent;
        break;
    /* Bright foreground 90-97 */
    case 90: case 91: case 92: case 93:
    case 94: case 95: case 96: case 97:
        m_fg = colorFromIndex(code - 90, true);
        break;
    /* Bright background 100-107 */
    case 100: case 101: case 102: case 103:
    case 104: case 105: case 106: case 107:
        m_bg = colorFromIndex(code - 100, true);
        break;
    default:
        break;
    }
}

void AnsiParser::processCSI(char finalByte) {
    QStringList params = m_paramBuffer.split(QLatin1Char(';'));

    switch (finalByte) {
    case 'm': /* SGR - Select Graphic Rendition */
        if (params.isEmpty() || (params.size() == 1 && params[0].isEmpty())) {
            applySGR(0); /* No params = reset */
        } else {
            for (const auto &p : params) {
                bool ok = false;
                int code = p.toInt(&ok);
                if (ok) applySGR(code);
            }
        }
        break;
    case 'A': /* Cursor up */
        {
            int n = params.isEmpty() ? 1 : qMax(1, params[0].toInt());
            m_cursorRow = qMax(0, m_cursorRow - n);
        }
        break;
    case 'B': /* Cursor down */
        {
            int n = params.isEmpty() ? 1 : qMax(1, params[0].toInt());
            m_cursorRow += n;
        }
        break;
    case 'C': /* Cursor forward */
        {
            int n = params.isEmpty() ? 1 : qMax(1, params[0].toInt());
            m_cursorCol += n;
        }
        break;
    case 'D': /* Cursor back */
        {
            int n = params.isEmpty() ? 1 : qMax(1, params[0].toInt());
            m_cursorCol = qMax(0, m_cursorCol - n);
        }
        break;
    case 'H': /* Cursor position */
    case 'f':
        {
            int row = (params.size() > 0 && !params[0].isEmpty())
                          ? params[0].toInt() - 1 : 0;
            int col = (params.size() > 1 && !params[1].isEmpty())
                          ? params[1].toInt() - 1 : 0;
            m_cursorRow = qMax(0, row);
            m_cursorCol = qMax(0, col);
        }
        break;
    case 'J': /* Erase in display */
    case 'K': /* Erase in line */
        /* Handled at a higher level */
        break;
    default:
        break;
    }
}

void AnsiParser::processChar(char ch) {
    switch (m_state) {
    case Normal:
        if (ch == '\033') {
            /* Begin escape sequence */
            if (!m_currentSegment.text.isEmpty()) {
                m_currentSegment.foreground = m_fg;
                m_currentSegment.background = m_bg;
                m_currentSegment.bold = m_bold;
                m_currentSegment.italic = m_italic;
                m_currentSegment.underline = m_underline;
                m_currentSegment.inverse = m_inverse;
                m_currentLine.segments.append(m_currentSegment);
                m_currentSegment = {};
            }
            m_state = Escape;
        } else if (ch == '\n') {
            /* Newline: finish current line */
            if (!m_currentSegment.text.isEmpty()) {
                m_currentSegment.foreground = m_fg;
                m_currentSegment.background = m_bg;
                m_currentSegment.bold = m_bold;
                m_currentSegment.italic = m_italic;
                m_currentSegment.underline = m_underline;
                m_currentSegment.inverse = m_inverse;
                m_currentLine.segments.append(m_currentSegment);
                m_currentSegment = {};
            }
            m_lines.append(m_currentLine);
            m_currentLine = {};
            m_cursorRow++;
            m_cursorCol = 0;
        } else if (ch == '\r') {
            m_cursorCol = 0;
        } else if (ch == '\b') {
            if (m_cursorCol > 0) m_cursorCol--;
        } else if (ch == '\t') {
            int spaces = 8 - (m_cursorCol % 8);
            m_currentSegment.text += QString(spaces, QLatin1Char(' '));
            m_cursorCol += spaces;
        } else if (ch >= 0x20 || (ch < 0 && (ch & 0x80))) {
            /* Printable character or UTF-8 leading byte */
            m_currentSegment.text += QLatin1Char(ch);
            m_cursorCol++;
        }
        break;

    case Escape:
        if (ch == '[') {
            m_state = CSI;
            m_paramBuffer.clear();
        } else if (ch == ']') {
            m_state = OSC;
            m_paramBuffer.clear();
        } else {
            /* Unrecognized escape, return to Normal */
            m_state = Normal;
        }
        break;

    case CSI:
        if ((ch >= '0' && ch <= '9') || ch == ';' || ch == '?') {
            m_paramBuffer += QLatin1Char(ch);
        } else {
            /* Final byte of CSI sequence */
            processCSI(ch);
            m_state = Normal;
        }
        break;

    case OSC:
        if (ch == '\007' || ch == '\033') {
            /* End of OSC (BEL or ST) */
            m_state = Normal;
        }
        /* Accumulate but ignore OSC content */
        break;
    }
}

QVector<TerminalLine> AnsiParser::parse(const QByteArray &data) {
    m_lines.clear();
    m_currentLine = {};
    m_currentSegment = {};

    for (char ch : data) {
        processChar(ch);
    }

    /* Flush remaining segment/line */
    if (!m_currentSegment.text.isEmpty()) {
        m_currentSegment.foreground = m_fg;
        m_currentSegment.background = m_bg;
        m_currentSegment.bold = m_bold;
        m_currentSegment.italic = m_italic;
        m_currentSegment.underline = m_underline;
        m_currentSegment.inverse = m_inverse;
        m_currentLine.segments.append(m_currentSegment);
    }
    if (!m_currentLine.segments.isEmpty()) {
        m_lines.append(m_currentLine);
    }

    return m_lines;
}
