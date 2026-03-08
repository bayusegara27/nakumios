/*
 * MountManager - USB auto-mounting via org.freedesktop.UDisks2 D-Bus
 */

#ifndef MOUNTMANAGER_H
#define MOUNTMANAGER_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QQmlEngine>

struct MountPoint {
    QString device;
    QString label;
    QString mountPath;
    QString fsType;
    qint64 size;
    bool mounted;
};

class MountManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QStringList mountedDevices READ mountedDevices NOTIFY devicesChanged)

public:
    explicit MountManager(QObject *parent = nullptr);

    QStringList mountedDevices() const;

    Q_INVOKABLE void mount(const QString &devicePath);
    Q_INVOKABLE void unmount(const QString &devicePath);
    Q_INVOKABLE void refresh();

signals:
    void devicesChanged();
    void mounted(const QString &mountPath);
    void unmounted(const QString &devicePath);
    void errorOccurred(const QString &message);

private slots:
    void onInterfacesAdded(const QDBusMessage &message);
    void onInterfacesRemoved(const QDBusMessage &message);

private:
    void connectToUDisks();
    QVector<MountPoint> m_devices;
};

#endif // MOUNTMANAGER_H
