/**
 * NakumiOS Terminal Controller Implementation
 */

#include "terminalcontroller.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QDir>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <cstdlib>

namespace NakumiOS {

// Thread-safe static ANSI escape code regex
Q_GLOBAL_STATIC_WITH_ARGS(QRegularExpression, s_ansiRegex, 
    (QStringLiteral("\x1b\\[[0-9;]*[a-zA-Z]")))

TerminalController::TerminalController(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_running(false)
    , m_columns(80)
    , m_rows(24)
    , m_ptyFd(-1)
{
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &TerminalController::onReadyRead);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &TerminalController::onReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &TerminalController::onProcessFinished);
}

TerminalController::~TerminalController()
{
    stop();
}

QString TerminalController::output() const
{
    return m_output;
}

bool TerminalController::running() const
{
    return m_running;
}

QString TerminalController::shellPrompt() const
{
    return QStringLiteral("$ ");
}

QFont TerminalController::terminalFont() const
{
    QFont font;
    font.setFamily(QStringLiteral("JetBrains Mono, Fira Code, Monospace"));
    font.setPointSize(11);
    font.setStyleHint(QFont::Monospace);
    return font;
}

QColor TerminalController::foregroundColor() const
{
    return QColor(0xE0, 0xE0, 0xE0);
}

QColor TerminalController::backgroundColor() const
{
    return QColor(0x0F, 0x0F, 0x14);
}

void TerminalController::start(const QString &shell)
{
    if (m_running) {
        return;
    }
    
    QString shellPath = shell;
    if (shellPath.isEmpty()) {
        // Try to get user's default shell
        const char *envShell = std::getenv("SHELL");
        if (envShell) {
            shellPath = QString::fromLocal8Bit(envShell);
        } else {
            shellPath = QStringLiteral("/bin/bash");
        }
    }
    
    // Set up environment
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("TERM"), QStringLiteral("xterm-256color"));
    env.insert(QStringLiteral("COLORTERM"), QStringLiteral("truecolor"));
    env.insert(QStringLiteral("COLUMNS"), QString::number(m_columns));
    env.insert(QStringLiteral("LINES"), QString::number(m_rows));
    m_process->setProcessEnvironment(env);
    
    // Set working directory to home
    m_process->setWorkingDirectory(QDir::homePath());
    
    // Start the shell in interactive mode
    m_process->start(shellPath, QStringList() << QStringLiteral("-i"));
    
    if (m_process->waitForStarted(3000)) {
        m_running = true;
        emit runningChanged();
        
        appendOutput(QStringLiteral("NakumiOS Terminal\n"));
        appendOutput(QStringLiteral("Shell: ") + shellPath + QStringLiteral("\n\n"));
    } else {
        appendOutput(QStringLiteral("Error: Failed to start shell\n"));
    }
}

void TerminalController::stop()
{
    if (!m_running) {
        return;
    }
    
    m_process->terminate();
    if (!m_process->waitForFinished(1000)) {
        m_process->kill();
    }
    
    m_running = false;
    emit runningChanged();
}

void TerminalController::sendInput(const QString &text)
{
    if (!m_running) {
        return;
    }
    
    m_process->write(text.toUtf8());
}

void TerminalController::sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    if (!m_running) {
        return;
    }
    
    QByteArray sequence;
    
    // Handle control key combinations
    if (modifiers & Qt::ControlModifier) {
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            // Ctrl+A to Ctrl+Z are ASCII 1-26
            char c = static_cast<char>(key - Qt::Key_A + 1);
            sequence.append(c);
            m_process->write(sequence);
            return;
        }
    }
    
    // Handle special keys
    switch (key) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        sequence = "\n";
        break;
    case Qt::Key_Backspace:
        sequence = "\x7f";
        break;
    case Qt::Key_Tab:
        sequence = "\t";
        break;
    case Qt::Key_Escape:
        sequence = "\x1b";
        break;
    case Qt::Key_Up:
        sequence = "\x1b[A";
        break;
    case Qt::Key_Down:
        sequence = "\x1b[B";
        break;
    case Qt::Key_Right:
        sequence = "\x1b[C";
        break;
    case Qt::Key_Left:
        sequence = "\x1b[D";
        break;
    case Qt::Key_Home:
        sequence = "\x1b[H";
        break;
    case Qt::Key_End:
        sequence = "\x1b[F";
        break;
    case Qt::Key_PageUp:
        sequence = "\x1b[5~";
        break;
    case Qt::Key_PageDown:
        sequence = "\x1b[6~";
        break;
    case Qt::Key_Insert:
        sequence = "\x1b[2~";
        break;
    case Qt::Key_Delete:
        sequence = "\x1b[3~";
        break;
    default:
        return;
    }
    
    m_process->write(sequence);
}

void TerminalController::resize(int columns, int rows)
{
    m_columns = columns;
    m_rows = rows;
    
    // Update environment for child process
    QProcessEnvironment env = m_process->processEnvironment();
    env.insert(QStringLiteral("COLUMNS"), QString::number(columns));
    env.insert(QStringLiteral("LINES"), QString::number(rows));
    m_process->setProcessEnvironment(env);
}

void TerminalController::clear()
{
    m_output.clear();
    emit outputChanged();
}

void TerminalController::copy()
{
    // This would be called from QML with selected text
    // Implementation depends on selection handling in the view
}

void TerminalController::paste()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text = clipboard->text();
    if (!text.isEmpty()) {
        sendInput(text);
    }
}

void TerminalController::onReadyRead()
{
    QByteArray stdoutData = m_process->readAllStandardOutput();
    QByteArray stderrData = m_process->readAllStandardError();
    
    if (!stdoutData.isEmpty()) {
        QString text = QString::fromUtf8(stdoutData);
        text = processAnsiCodes(text);
        appendOutput(text);
    }
    
    if (!stderrData.isEmpty()) {
        QString text = QString::fromUtf8(stderrData);
        text = processAnsiCodes(text);
        appendOutput(text);
    }
}

void TerminalController::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status)
    
    m_running = false;
    emit runningChanged();
    
    appendOutput(QStringLiteral("\n[Process exited with code %1]\n").arg(exitCode));
    emit finished(exitCode);
}

void TerminalController::appendOutput(const QString &text)
{
    m_output += text;
    
    // Limit buffer size
    const int maxLength = 100000;
    if (m_output.length() > maxLength) {
        m_output = m_output.right(maxLength);
    }
    
    emit outputChanged();
    emit outputReceived(text);
}

QString TerminalController::processAnsiCodes(const QString &text)
{
    // Basic ANSI escape code handling
    // For a full terminal emulator, you'd want to use a proper library
    QString result = text;
    
    // Remove some common control sequences (simplified)
    result.remove(*s_ansiRegex);
    
    // Remove bell character
    result.remove(QChar(0x07));
    
    return result;
}

} // namespace NakumiOS
