/*
 * TerminalBackend implementation
 *
 * Uses forkpty() to spawn a shell in a pseudo-terminal,
 * and poll() to read output without blocking the Qt event loop.
 */

#include "terminalbackend.h"

#include <pty.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <QCoreApplication>

/* ======================================================================
 * PtyReader - Runs in a separate thread, reads from the PTY fd
 * ====================================================================== */

PtyReader::PtyReader(int fd, QObject *parent)
    : QObject(parent), m_fd(fd) {
}

void PtyReader::start() {
    m_running = true;
    char buf[4096];

    while (m_running) {
        struct pollfd pfd;
        pfd.fd = m_fd;
        pfd.events = POLLIN;

        int ret = poll(&pfd, 1, 100); /* 100ms timeout */
        if (ret > 0 && (pfd.revents & POLLIN)) {
            ssize_t n = read(m_fd, buf, sizeof(buf));
            if (n > 0) {
                emit dataReady(QByteArray(buf, n));
            } else if (n <= 0) {
                break; /* EOF or error */
            }
        } else if (ret < 0) {
            break; /* poll error */
        }
    }

    emit finished();
}

void PtyReader::stop() {
    m_running = false;
}

/* ======================================================================
 * TerminalBackend
 * ====================================================================== */

TerminalBackend::TerminalBackend(QObject *parent)
    : QObject(parent) {
}

TerminalBackend::~TerminalBackend() {
    if (m_reader) {
        m_reader->stop();
    }
    m_readerThread.quit();
    m_readerThread.wait(1000);

    if (m_masterFd >= 0) {
        close(m_masterFd);
    }
    if (m_childPid > 0) {
        kill(m_childPid, SIGTERM);
        waitpid(m_childPid, nullptr, WNOHANG);
    }
}

void TerminalBackend::start(const QString &shell) {
    if (m_running) return;

    /* Determine shell */
    QByteArray shellPath;
    if (!shell.isEmpty()) {
        shellPath = shell.toUtf8();
    } else {
        const char *envShell = getenv("SHELL");
        shellPath = envShell ? QByteArray(envShell) : QByteArray("/bin/bash");
    }

    struct winsize ws;
    ws.ws_row = m_rows;
    ws.ws_col = m_cols;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    m_childPid = forkpty(&m_masterFd, nullptr, nullptr, &ws);

    if (m_childPid < 0) {
        qWarning("TerminalBackend: forkpty() failed");
        return;
    }

    if (m_childPid == 0) {
        /* Child process: exec the shell */
        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        execl(shellPath.constData(), shellPath.constData(), "-l",
              static_cast<char *>(nullptr));
        _exit(1);
    }

    /* Parent: start reading from the PTY */
    m_running = true;
    emit runningChanged();

    m_reader = new PtyReader(m_masterFd);
    m_reader->moveToThread(&m_readerThread);

    connect(&m_readerThread, &QThread::started, m_reader, &PtyReader::start);
    connect(m_reader, &PtyReader::dataReady, this, &TerminalBackend::onPtyData);
    connect(m_reader, &PtyReader::finished, this, &TerminalBackend::onPtyFinished);
    connect(m_reader, &PtyReader::finished, &m_readerThread, &QThread::quit);
    connect(&m_readerThread, &QThread::finished, m_reader, &QObject::deleteLater);

    m_readerThread.start();
}

void TerminalBackend::sendInput(const QString &text) {
    if (m_masterFd < 0) return;
    QByteArray data = text.toUtf8();
    write(m_masterFd, data.constData(), data.size());
}

void TerminalBackend::sendKey(int key, int modifiers) {
    Q_UNUSED(modifiers)
    /* Map Qt keys to terminal escape sequences */
    QByteArray seq;
    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        seq = "\r";
        break;
    case Qt::Key_Backspace:
        seq = "\x7f";
        break;
    case Qt::Key_Tab:
        seq = "\t";
        break;
    case Qt::Key_Escape:
        seq = "\x1b";
        break;
    case Qt::Key_Up:
        seq = "\x1b[A";
        break;
    case Qt::Key_Down:
        seq = "\x1b[B";
        break;
    case Qt::Key_Right:
        seq = "\x1b[C";
        break;
    case Qt::Key_Left:
        seq = "\x1b[D";
        break;
    case Qt::Key_Home:
        seq = "\x1b[H";
        break;
    case Qt::Key_End:
        seq = "\x1b[F";
        break;
    case Qt::Key_Delete:
        seq = "\x1b[3~";
        break;
    case Qt::Key_PageUp:
        seq = "\x1b[5~";
        break;
    case Qt::Key_PageDown:
        seq = "\x1b[6~";
        break;
    default:
        return;
    }
    if (!seq.isEmpty() && m_masterFd >= 0) {
        write(m_masterFd, seq.constData(), seq.size());
    }
}

void TerminalBackend::resize(int cols, int rows) {
    m_cols = cols;
    m_rows = rows;
    updateWindowSize();
    emit sizeChanged();
}

void TerminalBackend::setRows(int rows) {
    if (m_rows != rows) {
        m_rows = rows;
        updateWindowSize();
        emit sizeChanged();
    }
}

void TerminalBackend::setCols(int cols) {
    if (m_cols != cols) {
        m_cols = cols;
        updateWindowSize();
        emit sizeChanged();
    }
}

void TerminalBackend::updateWindowSize() {
    if (m_masterFd < 0) return;
    struct winsize ws;
    ws.ws_row = m_rows;
    ws.ws_col = m_cols;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    ioctl(m_masterFd, TIOCSWINSZ, &ws);
}

void TerminalBackend::onPtyData(const QByteArray &data) {
    /* Parse ANSI and update output text */
    auto lines = m_parser.parse(data);
    QString text;
    for (const auto &line : lines) {
        for (const auto &seg : line.segments) {
            text += seg.text;
        }
        text += QLatin1Char('\n');
    }

    /* Append to scrollback */
    auto newLines = text.split(QLatin1Char('\n'));
    m_scrollback.append(newLines);

    /* Trim scrollback if needed */
    while (m_scrollback.size() > m_maxScrollback) {
        m_scrollback.removeFirst();
    }

    m_outputText = m_scrollback.join(QLatin1Char('\n'));
    emit outputChanged();

    /* Check for bell character */
    if (data.contains('\a')) {
        emit bell();
    }
}

void TerminalBackend::onPtyFinished() {
    m_running = false;
    m_reader = nullptr;
    emit runningChanged();

    /* Reap child process */
    if (m_childPid > 0) {
        waitpid(m_childPid, nullptr, WNOHANG);
        m_childPid = -1;
    }
}
