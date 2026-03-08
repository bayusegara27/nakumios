/**
 * NakumiOS Notification Service Implementation
 */

#include "notificationservice.h"

#include <QTimer>
#include <QDateTime>

namespace NakumiOS {

NotificationService::NotificationService(QObject *parent)
    : QObject(parent)
    , m_nextId(1)
{
}

NotificationService::~NotificationService()
{
}

bool NotificationService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    
    if (!bus.registerService(QStringLiteral("org.freedesktop.Notifications"))) {
        qWarning() << "Failed to register notification D-Bus service";
        return false;
    }
    
    if (!bus.registerObject(QStringLiteral("/org/freedesktop/Notifications"), this,
                            QDBusConnection::ExportScriptableSlots | 
                            QDBusConnection::ExportScriptableSignals)) {
        qWarning() << "Failed to register notification D-Bus object";
        return false;
    }
    
    return true;
}

QStringList NotificationService::GetCapabilities()
{
    return QStringList{
        QStringLiteral("body"),
        QStringLiteral("body-markup"),
        QStringLiteral("body-hyperlinks"),
        QStringLiteral("body-images"),
        QStringLiteral("icon-static"),
        QStringLiteral("actions"),
        QStringLiteral("persistence")
    };
}

quint32 NotificationService::Notify(const QString &app_name,
                                    quint32 replaces_id,
                                    const QString &app_icon,
                                    const QString &summary,
                                    const QString &body,
                                    const QStringList &actions,
                                    const QVariantMap &hints,
                                    qint32 expire_timeout)
{
    quint32 id;
    
    if (replaces_id > 0 && m_notifications.contains(replaces_id)) {
        id = replaces_id;
    } else {
        id = m_nextId++;
    }
    
    Notification notification;
    notification.id = id;
    notification.appName = app_name;
    notification.appIcon = app_icon;
    notification.summary = summary;
    notification.body = body;
    notification.actions = actions;
    notification.hints = hints;
    notification.timeout = expire_timeout;
    notification.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    m_notifications[id] = notification;
    
    // Emit for QML UI
    emit notificationReceived(id, app_name, summary, body, app_icon);
    
    // Set up expiration timer if timeout is positive
    if (expire_timeout > 0) {
        QTimer::singleShot(expire_timeout, this, [this, id]() {
            expireNotification(id);
        });
    } else if (expire_timeout == -1) {
        // Use default timeout of 5 seconds
        QTimer::singleShot(5000, this, [this, id]() {
            expireNotification(id);
        });
    }
    // timeout == 0 means notification never expires
    
    return id;
}

void NotificationService::CloseNotification(quint32 id)
{
    if (m_notifications.contains(id)) {
        m_notifications.remove(id);
        emit NotificationClosed(id, 3); // 3 = closed by call to CloseNotification
        emit notificationClosed(id);
    }
}

QString NotificationService::GetServerInformation(QString &vendor, QString &version, QString &spec_version)
{
    vendor = QStringLiteral("NakumiOS");
    version = QStringLiteral(NAKUMIOS_VERSION);
    spec_version = QStringLiteral("1.2");
    return QStringLiteral("NakumiOS Notification Server");
}

void NotificationService::expireNotification(quint32 id)
{
    if (m_notifications.contains(id)) {
        m_notifications.remove(id);
        emit NotificationClosed(id, 1); // 1 = expired
        emit notificationClosed(id);
    }
}

} // namespace NakumiOS
