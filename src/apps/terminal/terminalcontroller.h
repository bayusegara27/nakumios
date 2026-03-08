/**
 * NakumiOS Terminal Controller
 * 
 * PTY-based terminal emulator backend providing shell access.
 */

#ifndef NAKUMIOS_TERMINALCONTROLLER_H
#define NAKUMIOS_TERMINALCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QProcess>
#include <QSocketNotifier>
#include <QFont>
#include <QColor>

namespace NakumiOS {

class TerminalController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    
    Q_PROPERTY(QString output READ output NOTIFY outputChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString shellPrompt READ shellPrompt CONSTANT)
    Q_PROPERTY(QFont terminalFont READ terminalFont CONSTANT)
    Q_PROPERTY(QColor foregroundColor READ foregroundColor CONSTANT)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor CONSTANT)

public:
    explicit TerminalController(QObject *parent = nullptr);
    ~TerminalController() override;
    
    QString output() const;
    bool running() const;
    QString shellPrompt() const;
    QFont terminalFont() const;
    QColor foregroundColor() const;
    QColor backgroundColor() const;
    
    Q_INVOKABLE void start(const QString &shell = QString());
    Q_INVOKABLE void stop();
    Q_INVOKABLE void sendInput(const QString &text);
    Q_INVOKABLE void sendKey(Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    Q_INVOKABLE void resize(int columns, int rows);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void copy();
    Q_INVOKABLE void paste();

signals:
    void outputChanged();
    void runningChanged();
    void outputReceived(const QString &text);
    void finished(int exitCode);

private slots:
    void onReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void appendOutput(const QString &text);
    QString processAnsiCodes(const QString &text);
    
    QProcess *m_process;
    QString m_output;
    QString m_currentLine;
    bool m_running;
    int m_columns;
    int m_rows;
    int m_ptyFd;
};

} // namespace NakumiOS

#endif // NAKUMIOS_TERMINALCONTROLLER_H
