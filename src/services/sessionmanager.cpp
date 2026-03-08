/**
 * NakumiOS Session Manager Implementation
 */

#include "sessionmanager.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QStandardPaths>

namespace NakumiOS {

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_sessionState(QStringLiteral("active"))
{
    loadAutoStartApps();
}

SessionManager::~SessionManager()
{
    saveSessionState();
}

bool SessionManager::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    
    if (!bus.registerService(QStringLiteral("org.nakumios.SessionManager"))) {
        qWarning() << "Failed to register D-Bus service";
        return false;
    }
    
    if (!bus.registerObject(QStringLiteral("/org/nakumios/SessionManager"), this,
                            QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals)) {
        qWarning() << "Failed to register D-Bus object";
        return false;
    }
    
    return true;
}

void SessionManager::Logout()
{
    emit sessionEnding();
    
    // Use systemd-logind to terminate session
    QDBusInterface loginManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1/session/auto"),
        QStringLiteral("org.freedesktop.login1.Session"),
        QDBusConnection::systemBus()
    );
    
    if (loginManager.isValid()) {
        loginManager.call(QStringLiteral("Terminate"));
    }
}

void SessionManager::Shutdown()
{
    emit sessionEnding();
    
    QDBusInterface loginManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus()
    );
    
    if (loginManager.isValid()) {
        loginManager.call(QStringLiteral("PowerOff"), true);
    }
}

void SessionManager::Reboot()
{
    emit sessionEnding();
    
    QDBusInterface loginManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus()
    );
    
    if (loginManager.isValid()) {
        loginManager.call(QStringLiteral("Reboot"), true);
    }
}

void SessionManager::Suspend()
{
    QDBusInterface loginManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus()
    );
    
    if (loginManager.isValid()) {
        loginManager.call(QStringLiteral("Suspend"), true);
    }
}

void SessionManager::LockScreen()
{
    m_sessionState = QStringLiteral("locked");
    emit sessionLocked();
    
    QDBusInterface sessionManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1/session/auto"),
        QStringLiteral("org.freedesktop.login1.Session"),
        QDBusConnection::systemBus()
    );
    
    if (sessionManager.isValid()) {
        sessionManager.call(QStringLiteral("Lock"));
    }
}

QString SessionManager::GetSessionState()
{
    return m_sessionState;
}

QStringList SessionManager::GetAutoStartApps()
{
    return m_autoStartApps;
}

void SessionManager::loadAutoStartApps()
{
    m_autoStartApps.clear();
    
    QStringList searchPaths = {
        QStringLiteral("/etc/xdg/autostart"),
        QDir::homePath() + QStringLiteral("/.config/autostart")
    };
    
    for (const QString &path : searchPaths) {
        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }
        
        QStringList filters;
        filters << QStringLiteral("*.desktop");
        
        const QFileInfoList entries = dir.entryInfoList(filters, QDir::Files);
        for (const QFileInfo &entry : entries) {
            m_autoStartApps.append(entry.absoluteFilePath());
        }
    }
}

void SessionManager::launchAutoStartApps()
{
    for (const QString &desktopFile : m_autoStartApps) {
        QFile file(desktopFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        
        QString exec;
        bool hidden = false;
        
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            
            if (line.startsWith(QLatin1String("Exec="))) {
                exec = line.mid(5);
            } else if (line.startsWith(QLatin1String("Hidden="))) {
                hidden = (line.mid(7).toLower() == QLatin1String("true"));
            }
        }
        file.close();
        
        if (!hidden && !exec.isEmpty()) {
            // Remove field codes
            exec.remove(QRegularExpression(QStringLiteral("%[fFuUdDnNickvm]")));
            exec = exec.trimmed();
            
            QProcess::startDetached(
                QStringLiteral("/bin/sh"),
                QStringList() << QStringLiteral("-c") << exec
            );
        }
    }
}

void SessionManager::saveSessionState()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configPath + QStringLiteral("/nakumios"));
    
    QFile file(configPath + QStringLiteral("/nakumios/session.state"));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "state=" << m_sessionState << "\n";
        file.close();
    }
}

} // namespace NakumiOS
