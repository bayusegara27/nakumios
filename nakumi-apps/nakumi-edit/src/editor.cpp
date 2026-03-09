/*
 * Editor implementation
 */

#include "editor.h"
#include "linenumberarea.h"

#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QTextBlock>

Editor::Editor(QWidget *parent)
    : QPlainTextEdit(parent) {
    setupEditor();
}

void Editor::setupEditor() {
    /* NakumiOS dark theme */
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(0x1A, 0x1A, 0x24));
    p.setColor(QPalette::Text, QColor(0xE2, 0xE8, 0xF0));
    setPalette(p);

    QFont font(QStringLiteral("monospace"), 12);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);

    /* Line number area */
    m_lineNumberArea = new LineNumberArea(this);
    m_highlighter = new SyntaxHighlighter(document());

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &Editor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &Editor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &Editor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int Editor::lineNumberAreaWidth() const {
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void Editor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
}

void Editor::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(0x2D, 0x2D, 0x3F));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(0x12, 0x12, 0x1A));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(0x60, 0x60, 0x80));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void Editor::openFile(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit statusMessage(QStringLiteral("Cannot open: ") + path);
        return;
    }
    setPlainText(QString::fromUtf8(file.readAll()));
    m_currentFile = path;
    document()->setModified(false);

    /* Auto-detect language */
    m_highlighter->setLanguage(SyntaxHighlighter::detectLanguage(path));

    emit fileChanged(path);
    emit statusMessage(QStringLiteral("Opened: ") + path);
}

void Editor::saveFile() {
    if (m_currentFile.isEmpty()) return;
    saveFileAs(m_currentFile);
}

void Editor::saveFileAs(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit statusMessage(QStringLiteral("Cannot save: ") + path);
        return;
    }
    file.write(toPlainText().toUtf8());
    m_currentFile = path;
    document()->setModified(false);
    emit fileChanged(path);
    emit statusMessage(QStringLiteral("Saved: ") + path);
}
