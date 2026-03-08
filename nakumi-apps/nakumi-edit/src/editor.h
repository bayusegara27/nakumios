/*
 * Editor - QPlainTextEdit with line numbers and syntax highlighting
 */

#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>
#include "syntaxhighlighter.h"

class LineNumberArea;

class Editor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    void openFile(const QString &path);
    void saveFile();
    void saveFileAs(const QString &path);

    QString currentFile() const { return m_currentFile; }
    bool isModified() const { return document()->isModified(); }

signals:
    void fileChanged(const QString &path);
    void statusMessage(const QString &msg);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
    void setupEditor();
    LineNumberArea *m_lineNumberArea;
    SyntaxHighlighter *m_highlighter;
    QString m_currentFile;
};

#endif // EDITOR_H
