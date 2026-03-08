/**
 * NakumiOS Shell Controller Implementation
 */

#include "shellcontroller.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QSettings>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <algorithm>

namespace NakumiOS {

ShellController::ShellController(QObject *parent)
    : QObject(parent)
    , m_launcherVisible(false)
{
    loadApplications();
    loadPinnedApps();
}

ShellController::~ShellController()
{
    savePinnedApps();
}

ShellController *ShellController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(jsEngine)
    
    auto *controller = new ShellController();
    QQmlEngine::setObjectOwnership(controller, QQmlEngine::JavaScriptOwnership);
    
    return controller;
}

QVariantList ShellController::applications() const
{
    QVariantList list;
    const QList<AppInfo> &apps = m_filteredApplications.isEmpty() 
        ? m_applications 
        : m_filteredApplications;
    
    for (const AppInfo &app : apps) {
        QVariantMap map;
        map[QStringLiteral("name")] = app.name;
        map[QStringLiteral("exec")] = app.exec;
        map[QStringLiteral("icon")] = app.icon;
        map[QStringLiteral("categories")] = app.categories;
        map[QStringLiteral("comment")] = app.comment;
        map[QStringLiteral("desktopFile")] = app.desktopFile;
        map[QStringLiteral("terminal")] = app.terminal;
        list.append(map);
    }
    
    return list;
}

QVariantList ShellController::pinnedApps() const
{
    QVariantList list;
    
    for (const QString &desktopFile : m_pinnedDesktopFiles) {
        for (const AppInfo &app : m_applications) {
            if (app.desktopFile == desktopFile) {
                QVariantMap map;
                map[QStringLiteral("name")] = app.name;
                map[QStringLiteral("exec")] = app.exec;
                map[QStringLiteral("icon")] = app.icon;
                map[QStringLiteral("desktopFile")] = app.desktopFile;
                list.append(map);
                break;
            }
        }
    }
    
    return list;
}

bool ShellController::launcherVisible() const
{
    return m_launcherVisible;
}

void ShellController::setLauncherVisible(bool visible)
{
    if (m_launcherVisible != visible) {
        m_launcherVisible = visible;
        emit launcherVisibleChanged();
    }
}

void ShellController::launchApplication(const QString &desktopFile)
{
    for (const AppInfo &app : m_applications) {
        if (app.desktopFile == desktopFile) {
            QString exec = app.exec;
            
            // Remove field codes from exec string
            exec.remove(QRegularExpression(QStringLiteral("%[fFuUdDnNickvm]")));
            exec = exec.trimmed();
            
            if (app.terminal) {
                exec = QStringLiteral("nakumi-terminal -e ") + exec;
            }
            
            bool success = QProcess::startDetached(
                QStringLiteral("/bin/sh"),
                QStringList() << QStringLiteral("-c") << exec
            );
            
            if (success) {
                emit applicationLaunched(app.name);
            } else {
                emit errorOccurred(QStringLiteral("Failed to launch: ") + app.name);
            }
            
            setLauncherVisible(false);
            return;
        }
    }
    
    emit errorOccurred(QStringLiteral("Application not found: ") + desktopFile);
}

void ShellController::launchCommand(const QString &command)
{
    bool success = QProcess::startDetached(
        QStringLiteral("/bin/sh"),
        QStringList() << QStringLiteral("-c") << command
    );
    
    if (!success) {
        emit errorOccurred(QStringLiteral("Failed to execute: ") + command);
    }
    
    setLauncherVisible(false);
}

void ShellController::searchApplications(const QString &query)
{
    m_filteredApplications.clear();
    
    if (query.isEmpty()) {
        emit applicationsChanged();
        return;
    }
    
    QString lowerQuery = query.toLower();
    
    for (const AppInfo &app : m_applications) {
        if (app.name.toLower().contains(lowerQuery) ||
            app.comment.toLower().contains(lowerQuery) ||
            app.categories.toLower().contains(lowerQuery)) {
            m_filteredApplications.append(app);
        }
    }
    
    emit applicationsChanged();
}

void ShellController::logout()
{
    QDBusInterface sessionManager(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1/session/auto"),
        QStringLiteral("org.freedesktop.login1.Session"),
        QDBusConnection::systemBus()
    );
    
    if (sessionManager.isValid()) {
        sessionManager.call(QStringLiteral("Terminate"));
    }
}

void ShellController::shutdown()
{
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

void ShellController::reboot()
{
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

void ShellController::suspend()
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

void ShellController::lockScreen()
{
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

void ShellController::pinToDock(const QString &desktopFile)
{
    if (!m_pinnedDesktopFiles.contains(desktopFile)) {
        m_pinnedDesktopFiles.append(desktopFile);
        savePinnedApps();
        emit pinnedAppsChanged();
    }
}

void ShellController::unpinFromDock(const QString &desktopFile)
{
    if (m_pinnedDesktopFiles.removeAll(desktopFile) > 0) {
        savePinnedApps();
        emit pinnedAppsChanged();
    }
}

void ShellController::loadApplications()
{
    m_applications.clear();
    
    QStringList searchPaths = {
        QStringLiteral("/usr/share/applications"),
        QStringLiteral("/usr/local/share/applications"),
        QDir::homePath() + QStringLiteral("/.local/share/applications")
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
            AppInfo app = parseDesktopFile(entry.absoluteFilePath());
            if (!app.name.isEmpty() && !app.exec.isEmpty()) {
                m_applications.append(app);
            }
        }
    }
    
    // Sort alphabetically
    std::sort(m_applications.begin(), m_applications.end(),
              [](const AppInfo &a, const AppInfo &b) {
        return a.name.toLower() < b.name.toLower();
    });
    
    emit applicationsChanged();
}

void ShellController::loadPinnedApps()
{
    QSettings settings(QStringLiteral("NakumiOS"), QStringLiteral("Shell"));
    m_pinnedDesktopFiles = settings.value(
        QStringLiteral("pinnedApps"),
        QStringList() << QStringLiteral("org.nakumios.settings.desktop")
                      << QStringLiteral("org.nakumios.files.desktop")
                      << QStringLiteral("org.nakumios.terminal.desktop")
    ).toStringList();
    
    emit pinnedAppsChanged();
}

void ShellController::savePinnedApps()
{
    QSettings settings(QStringLiteral("NakumiOS"), QStringLiteral("Shell"));
    settings.setValue(QStringLiteral("pinnedApps"), m_pinnedDesktopFiles);
}

AppInfo ShellController::parseDesktopFile(const QString &path)
{
    AppInfo app;
    app.desktopFile = QFileInfo(path).fileName();
    app.terminal = false;
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return app;
    }
    
    QTextStream stream(&file);
    bool inDesktopEntry = false;
    bool noDisplay = false;
    bool hidden = false;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        
        if (line.startsWith(QLatin1Char('['))) {
            inDesktopEntry = (line == QLatin1String("[Desktop Entry]"));
            continue;
        }
        
        if (!inDesktopEntry) {
            continue;
        }
        
        int equalsPos = line.indexOf(QLatin1Char('='));
        if (equalsPos < 0) {
            continue;
        }
        
        QString key = line.left(equalsPos).trimmed();
        QString value = line.mid(equalsPos + 1).trimmed();
        
        if (key == QLatin1String("Name")) {
            if (app.name.isEmpty()) { // Only set if not already set (localized versions come after)
                app.name = value;
            }
        } else if (key == QLatin1String("Exec")) {
            app.exec = value;
        } else if (key == QLatin1String("Icon")) {
            app.icon = value;
        } else if (key == QLatin1String("Categories")) {
            app.categories = value;
        } else if (key == QLatin1String("Comment")) {
            if (app.comment.isEmpty()) {
                app.comment = value;
            }
        } else if (key == QLatin1String("Terminal")) {
            app.terminal = (value.toLower() == QLatin1String("true"));
        } else if (key == QLatin1String("NoDisplay")) {
            noDisplay = (value.toLower() == QLatin1String("true"));
        } else if (key == QLatin1String("Hidden")) {
            hidden = (value.toLower() == QLatin1String("true"));
        }
    }
    
    file.close();
    
    // Don't include hidden or no-display apps
    if (noDisplay || hidden) {
        app.name.clear();
    }
    
    return app;
}

} // namespace NakumiOS
