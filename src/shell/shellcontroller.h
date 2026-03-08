/**
 * NakumiOS Shell Controller
 * 
 * Backend controller for the desktop shell providing
 * application launching, system actions, and D-Bus integration.
 */

#ifndef NAKUMIOS_SHELLCONTROLLER_H
#define NAKUMIOS_SHELLCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QStringList>
#include <QVariantList>
#include <QProcess>

namespace NakumiOS {

struct AppInfo {
    QString name;
    QString exec;
    QString icon;
    QString categories;
    QString comment;
    QString desktopFile;
    bool terminal;
};

class ShellController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(QVariantList applications READ applications NOTIFY applicationsChanged)
    Q_PROPERTY(QVariantList pinnedApps READ pinnedApps NOTIFY pinnedAppsChanged)
    Q_PROPERTY(bool launcherVisible READ launcherVisible WRITE setLauncherVisible NOTIFY launcherVisibleChanged)

public:
    explicit ShellController(QObject *parent = nullptr);
    ~ShellController() override;
    
    static ShellController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    QVariantList applications() const;
    QVariantList pinnedApps() const;
    bool launcherVisible() const;
    void setLauncherVisible(bool visible);
    
    Q_INVOKABLE void launchApplication(const QString &desktopFile);
    Q_INVOKABLE void launchCommand(const QString &command);
    Q_INVOKABLE void searchApplications(const QString &query);
    
    // System actions
    Q_INVOKABLE void logout();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void suspend();
    Q_INVOKABLE void lockScreen();
    
    // Dock management
    Q_INVOKABLE void pinToDock(const QString &desktopFile);
    Q_INVOKABLE void unpinFromDock(const QString &desktopFile);

signals:
    void applicationsChanged();
    void pinnedAppsChanged();
    void launcherVisibleChanged();
    void applicationLaunched(const QString &name);
    void errorOccurred(const QString &error);

private:
    void loadApplications();
    void loadPinnedApps();
    void savePinnedApps();
    AppInfo parseDesktopFile(const QString &path);
    
    QList<AppInfo> m_applications;
    QList<AppInfo> m_filteredApplications;
    QStringList m_pinnedDesktopFiles;
    bool m_launcherVisible;
};

} // namespace NakumiOS

#endif // NAKUMIOS_SHELLCONTROLLER_H
