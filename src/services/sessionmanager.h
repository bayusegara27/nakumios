/**
 * NakumiOS Session Manager
 * 
 * Manages user session lifecycle, startup applications,
 * and session state persistence.
 */

#ifndef NAKUMIOS_SESSIONMANAGER_H
#define NAKUMIOS_SESSIONMANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QStringList>

namespace NakumiOS {

class SessionManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nakumios.SessionManager")

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;
    
    bool registerService();
    
public slots:
    Q_SCRIPTABLE void Logout();
    Q_SCRIPTABLE void Shutdown();
    Q_SCRIPTABLE void Reboot();
    Q_SCRIPTABLE void Suspend();
    Q_SCRIPTABLE void LockScreen();
    Q_SCRIPTABLE QString GetSessionState();
    Q_SCRIPTABLE QStringList GetAutoStartApps();
    
signals:
    void sessionEnding();
    void sessionLocked();
    void sessionUnlocked();

private:
    void loadAutoStartApps();
    void launchAutoStartApps();
    void saveSessionState();
    
    QStringList m_autoStartApps;
    QString m_sessionState;
};

} // namespace NakumiOS

#endif // NAKUMIOS_SESSIONMANAGER_H
