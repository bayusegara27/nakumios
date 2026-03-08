/*
 * MountManager implementation - UDisks2 D-Bus integration
 */

#include "mountmanager.h"

#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusReply>

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
    QDBusInterface iface(UDISKS2_SERVICE, devicePath, UDISKS2_FS,
                         QDBusConnection::systemBus());
    if (!iface.isValid()) {
        emit errorOccurred(QStringLiteral("Invalid device: ") + devicePath);
        return;
    }

    QVariantMap options;
    QDBusReply<QString> reply = iface.call(QStringLiteral("Mount"), options);
    if (reply.isValid()) {
        emit mounted(reply.value());
        refresh();
    } else {
        emit errorOccurred(QStringLiteral("Mount failed: ") + reply.error().message());
    }
}

void MountManager::unmount(const QString &devicePath) {
    QDBusInterface iface(UDISKS2_SERVICE, devicePath, UDISKS2_FS,
                         QDBusConnection::systemBus());
    if (!iface.isValid()) {
        emit errorOccurred(QStringLiteral("Invalid device: ") + devicePath);
        return;
    }

    QVariantMap options;
    QDBusReply<void> reply = iface.call(QStringLiteral("Unmount"), options);
    if (reply.isValid()) {
        emit unmounted(devicePath);
        refresh();
    } else {
        emit errorOccurred(QStringLiteral("Unmount failed: ") + reply.error().message());
    }
}

void MountManager::refresh() {
    m_devices.clear();

    QDBusInterface objMgr(UDISKS2_SERVICE, UDISKS2_PATH, DBUS_OBJMGR,
                          QDBusConnection::systemBus());
    if (!objMgr.isValid()) {
        return;
    }

    QDBusMessage reply = objMgr.call(QStringLiteral("GetManagedObjects"));
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
        return;
    }

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
