/*
 * MountManager implementation - UDisks2 D-Bus integration
 *
 * All D-Bus calls use QDBusPendingCallWatcher for async execution
 * to avoid blocking the Qt main/GUI thread.
 */

#include "mountmanager.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusPendingReply>

static const auto UDISKS2_SERVICE = QStringLiteral("org.freedesktop.UDisks2");
static const auto UDISKS2_PATH = QStringLiteral("/org/freedesktop/UDisks2");
static const auto DBUS_OBJMGR = QStringLiteral("org.freedesktop.DBus.ObjectManager");
static const auto UDISKS2_FS = QStringLiteral("org.freedesktop.UDisks2.Filesystem");
static const auto UDISKS2_BLOCK = QStringLiteral("org.freedesktop.UDisks2.Block");

MountManager::MountManager(QObject *parent)
    : QObject(parent) {
    connectToUDisks();
    refresh();
}

void MountManager::connectToUDisks() {
    auto bus = QDBusConnection::systemBus();

    /* Listen for new/removed devices */
    bus.connect(UDISKS2_SERVICE, UDISKS2_PATH, DBUS_OBJMGR,
                QStringLiteral("InterfacesAdded"), this,
                SLOT(onInterfacesAdded(QDBusMessage)));
    bus.connect(UDISKS2_SERVICE, UDISKS2_PATH, DBUS_OBJMGR,
                QStringLiteral("InterfacesRemoved"), this,
                SLOT(onInterfacesRemoved(QDBusMessage)));
}

QStringList MountManager::mountedDevices() const {
    QStringList result;
    for (const auto &dev : m_devices) {
        QString label = dev.label.isEmpty() ? dev.device : dev.label;
        result.append(label);
    }
    return result;
}

void MountManager::mount(const QString &devicePath) {
    QDBusMessage msg = QDBusMessage::createMethodCall(
        UDISKS2_SERVICE, devicePath, UDISKS2_FS, QStringLiteral("Mount"));

    /* Construct the a{sv} options argument */
    QVariantMap options;
    msg << QVariant::fromValue(options);

    QDBusPendingCall pending =
        QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &MountManager::onMountFinished);
}

void MountManager::onMountFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QString> reply = *watcher;
    if (reply.isError()) {
        emit errorOccurred(QStringLiteral("Mount failed: ") +
                           reply.error().message());
    } else {
        emit mounted(reply.value());
        refresh();
    }
    watcher->deleteLater();
}

void MountManager::unmount(const QString &devicePath) {
    QDBusMessage msg = QDBusMessage::createMethodCall(
        UDISKS2_SERVICE, devicePath, UDISKS2_FS, QStringLiteral("Unmount"));

    /* Construct the a{sv} options argument */
    QVariantMap options;
    msg << QVariant::fromValue(options);

    QDBusPendingCall pending =
        QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    watcher->setProperty("devicePath", devicePath);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &MountManager::onUnmountFinished);
}

void MountManager::onUnmountFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<> reply = *watcher;
    if (reply.isError()) {
        emit errorOccurred(QStringLiteral("Unmount failed: ") +
                           reply.error().message());
    } else {
        emit unmounted(watcher->property("devicePath").toString());
        refresh();
    }
    watcher->deleteLater();
}

void MountManager::refresh() {
    QDBusMessage msg = QDBusMessage::createMethodCall(
        UDISKS2_SERVICE, UDISKS2_PATH, DBUS_OBJMGR,
        QStringLiteral("GetManagedObjects"));

    QDBusPendingCall pending =
        QDBusConnection::systemBus().asyncCall(msg);
    auto *watcher = new QDBusPendingCallWatcher(pending, this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &MountManager::onRefreshFinished);
}

void MountManager::onRefreshFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QDBusMessage> reply = *watcher;
    if (!reply.isError()) {
        QDBusMessage msg = reply.reply();
        parseRefreshReply(msg);
    }
    watcher->deleteLater();
}

void MountManager::parseRefreshReply(const QDBusMessage &reply) {
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
        return;
    }

    m_devices.clear();

    /* Parse managed objects for filesystems */
    const QDBusArgument arg = reply.arguments().first().value<QDBusArgument>();
    arg.beginMap();
    while (!arg.atEnd()) {
        arg.beginMapEntry();
        QDBusObjectPath objPath;
        arg >> objPath;

        QVariantMap interfaces;
        const QDBusArgument ifaceArg = arg.asVariant().value<QDBusArgument>();
        ifaceArg.beginMap();
        while (!ifaceArg.atEnd()) {
            ifaceArg.beginMapEntry();
            QString ifaceName;
            ifaceArg >> ifaceName;
            QVariantMap props;
            const QDBusArgument propsArg = ifaceArg.asVariant().value<QDBusArgument>();
            propsArg.beginMap();
            while (!propsArg.atEnd()) {
                propsArg.beginMapEntry();
                QString propName;
                QVariant propVal;
                propsArg >> propName >> propVal;
                props[propName] = propVal;
                propsArg.endMapEntry();
            }
            propsArg.endMap();
            interfaces[ifaceName] = props;
            ifaceArg.endMapEntry();
        }
        ifaceArg.endMap();

        if (interfaces.contains(UDISKS2_FS)) {
            MountPoint mp;
            mp.device = objPath.path();
            auto blockProps = interfaces.value(UDISKS2_BLOCK).toMap();
            mp.label = blockProps.value(QStringLiteral("IdLabel")).toString();
            mp.fsType = blockProps.value(QStringLiteral("IdType")).toString();
            mp.size = blockProps.value(QStringLiteral("Size")).toLongLong();

            auto fsProps = interfaces.value(UDISKS2_FS).toMap();
            auto mountPoints = fsProps.value(QStringLiteral("MountPoints")).toList();
            mp.mounted = !mountPoints.isEmpty();
            if (mp.mounted && !mountPoints.isEmpty()) {
                mp.mountPath = mountPoints.first().toString();
            }
            m_devices.append(mp);
        }

        arg.endMapEntry();
    }
    arg.endMap();

    emit devicesChanged();
}

void MountManager::onInterfacesAdded(const QDBusMessage &message) {
    Q_UNUSED(message)
    refresh();
}

void MountManager::onInterfacesRemoved(const QDBusMessage &message) {
    Q_UNUSED(message)
    refresh();
}
