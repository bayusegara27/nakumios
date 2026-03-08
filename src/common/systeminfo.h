/**
 * NakumiOS System Information Provider
 * 
 * Provides real-time system information including CPU usage,
 * memory usage, disk space, battery status, and network info.
 * Uses direct Linux /proc and /sys filesystem access for accuracy.
 */

#ifndef NAKUMIOS_SYSTEMINFO_H
#define NAKUMIOS_SYSTEMINFO_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QQmlEngine>

namespace NakumiOS {

class SystemInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(double cpuUsage READ cpuUsage NOTIFY cpuUsageChanged)
    Q_PROPERTY(qint64 totalMemory READ totalMemory CONSTANT)
    Q_PROPERTY(qint64 usedMemory READ usedMemory NOTIFY memoryChanged)
    Q_PROPERTY(qint64 availableMemory READ availableMemory NOTIFY memoryChanged)
    Q_PROPERTY(double memoryUsagePercent READ memoryUsagePercent NOTIFY memoryChanged)
    
    Q_PROPERTY(qint64 totalDisk READ totalDisk CONSTANT)
    Q_PROPERTY(qint64 usedDisk READ usedDisk NOTIFY diskChanged)
    Q_PROPERTY(qint64 availableDisk READ availableDisk NOTIFY diskChanged)
    Q_PROPERTY(double diskUsagePercent READ diskUsagePercent NOTIFY diskChanged)
    
    Q_PROPERTY(bool hasBattery READ hasBattery CONSTANT)
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY batteryChanged)
    Q_PROPERTY(bool batteryCharging READ batteryCharging NOTIFY batteryChanged)
    
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT)
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(qint64 uptime READ uptime NOTIFY uptimeChanged)

public:
    explicit SystemInfo(QObject *parent = nullptr);
    ~SystemInfo() override;
    
    static SystemInfo *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    // CPU
    double cpuUsage() const;
    
    // Memory
    qint64 totalMemory() const;
    qint64 usedMemory() const;
    qint64 availableMemory() const;
    double memoryUsagePercent() const;
    
    // Disk
    qint64 totalDisk() const;
    qint64 usedDisk() const;
    qint64 availableDisk() const;
    double diskUsagePercent() const;
    
    // Battery
    bool hasBattery() const;
    int batteryPercent() const;
    bool batteryCharging() const;
    
    // System
    QString hostname() const;
    QString kernelVersion() const;
    QString osName() const;
    qint64 uptime() const;
    
    Q_INVOKABLE void startMonitoring(int intervalMs = 1000);
    Q_INVOKABLE void stopMonitoring();
    Q_INVOKABLE QString formatBytes(qint64 bytes) const;
    Q_INVOKABLE QString formatUptime(qint64 seconds) const;

signals:
    void cpuUsageChanged();
    void memoryChanged();
    void diskChanged();
    void batteryChanged();
    void uptimeChanged();

private slots:
    void updateStats();

private:
    void readCpuStats();
    void readMemoryStats();
    void readDiskStats();
    void readBatteryStats();
    void readSystemInfo();
    
    QTimer *m_updateTimer;
    
    // CPU tracking
    qint64 m_prevCpuTotal;
    qint64 m_prevCpuIdle;
    double m_cpuUsage;
    
    // Memory
    qint64 m_totalMemory;
    qint64 m_availableMemory;
    
    // Disk
    qint64 m_totalDisk;
    qint64 m_availableDisk;
    
    // Battery
    bool m_hasBattery;
    int m_batteryPercent;
    bool m_batteryCharging;
    
    // System info
    QString m_hostname;
    QString m_kernelVersion;
    QString m_osName;
};

} // namespace NakumiOS

#endif // NAKUMIOS_SYSTEMINFO_H
