/*
 * LineNumberArea implementation
 */

#include "linenumberarea.h"
#include "editor.h"

LineNumberArea::LineNumberArea(Editor *editor)
    : QWidget(editor), m_editor(editor) {
}

QSize LineNumberArea::sizeHint() const {
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    m_editor->lineNumberAreaPaintEvent(event);
}
