/**
 * NakumiOS Notification Service
 * 
 * Implements freedesktop.org notification specification for
 * receiving and displaying desktop notifications.
 */

#ifndef NAKUMIOS_NOTIFICATIONSERVICE_H
#define NAKUMIOS_NOTIFICATIONSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QVariantMap>
#include <QQueue>

namespace NakumiOS {

struct Notification {
    quint32 id;
    QString appName;
    QString appIcon;
    QString summary;
    QString body;
    QStringList actions;
    QVariantMap hints;
    qint32 timeout;
    qint64 timestamp;
};

class NotificationService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    explicit NotificationService(QObject *parent = nullptr);
    ~NotificationService() override;
    
    bool registerService();

public slots:
    // D-Bus interface methods (freedesktop.org notifications spec)
    Q_SCRIPTABLE QStringList GetCapabilities();
    Q_SCRIPTABLE quint32 Notify(const QString &app_name,
                                quint32 replaces_id,
                                const QString &app_icon,
                                const QString &summary,
                                const QString &body,
                                const QStringList &actions,
                                const QVariantMap &hints,
                                qint32 expire_timeout);
    Q_SCRIPTABLE void CloseNotification(quint32 id);
    Q_SCRIPTABLE QString GetServerInformation(QString &vendor, QString &version, QString &spec_version);

signals:
    // D-Bus signals
    Q_SCRIPTABLE void NotificationClosed(quint32 id, quint32 reason);
    Q_SCRIPTABLE void ActionInvoked(quint32 id, const QString &action_key);
    
    // Internal signals for QML
    void notificationReceived(quint32 id, const QString &appName, const QString &summary, 
                             const QString &body, const QString &icon);
    void notificationClosed(quint32 id);

private:
    void expireNotification(quint32 id);
    
    QHash<quint32, Notification> m_notifications;
    quint32 m_nextId;
};

} // namespace NakumiOS

#endif // NAKUMIOS_NOTIFICATIONSERVICE_H
