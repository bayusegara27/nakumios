/*
 * TerminalBackend - PTY management for NakumiTerm
 *
 * Uses forkpty() to create a pseudo-terminal and poll() for non-blocking
 * I/O. Exposes the ANSI output stream to QML via signals.
 */

#ifndef TERMINALBACKEND_H
#define TERMINALBACKEND_H

#include <QObject>
#include <QThread>
#include <QQmlEngine>
#include <QStringList>

#include "ansiparser.h"

class PtyReader : public QObject {
    Q_OBJECT

public:
    explicit PtyReader(int fd, QObject *parent = nullptr);

public slots:
    void start();
    void stop();

signals:
    void dataReady(const QByteArray &data);
    void finished();

private:
    int m_fd;
    bool m_running = false;
};

class TerminalBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString outputText READ outputText NOTIFY outputChanged)
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY sizeChanged)
    Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY sizeChanged)
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    explicit TerminalBackend(QObject *parent = nullptr);
    ~TerminalBackend() override;

    QString outputText() const { return m_outputText; }
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    bool isRunning() const { return m_running; }

    void setRows(int rows);
    void setCols(int cols);

    Q_INVOKABLE void start(const QString &shell = QString());
    Q_INVOKABLE void sendInput(const QString &text);
    Q_INVOKABLE void sendKey(int key, int modifiers);
    Q_INVOKABLE void resize(int cols, int rows);

signals:
    void outputChanged();
    void sizeChanged();
    void runningChanged();
    void bell();

private slots:
    void onPtyData(const QByteArray &data);
    void onPtyFinished();

private:
    void updateWindowSize();

    int m_masterFd = -1;
    pid_t m_childPid = -1;
    int m_rows = 24;
    int m_cols = 80;
    bool m_running = false;

    QString m_outputText;
    QStringList m_scrollback;
    int m_maxScrollback = 10000;

    AnsiParser m_parser;
    QThread m_readerThread;
    PtyReader *m_reader = nullptr;
};

#endif // TERMINALBACKEND_H
