/**
 * NakumiOS System Information Provider Implementation
 * 
 * Reads system information directly from Linux /proc and /sys filesystems
 * for accurate real-time data without external dependencies.
 */

#include "systeminfo.h"

#include <QFile>
#include <QTextStream>
#include <QStorageInfo>
#include <QDir>
#include <QFileInfo>
#include <cmath>

namespace NakumiOS {

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent)
    , m_updateTimer(new QTimer(this))
    , m_prevCpuTotal(0)
    , m_prevCpuIdle(0)
    , m_cpuUsage(0.0)
    , m_totalMemory(0)
    , m_availableMemory(0)
    , m_totalDisk(0)
    , m_availableDisk(0)
    , m_hasBattery(false)
    , m_batteryPercent(0)
    , m_batteryCharging(false)
{
    connect(m_updateTimer, &QTimer::timeout, this, &SystemInfo::updateStats);
    
    // Initial system info read (static values)
    readSystemInfo();
    
    // Initial stats read
    updateStats();
}

SystemInfo::~SystemInfo()
{
    stopMonitoring();
}

SystemInfo *SystemInfo::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(jsEngine)
    
    auto *info = new SystemInfo();
    QQmlEngine::setObjectOwnership(info, QQmlEngine::JavaScriptOwnership);
    
    return info;
}

void SystemInfo::startMonitoring(int intervalMs)
{
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start(intervalMs);
    }
}

void SystemInfo::stopMonitoring()
{
    m_updateTimer->stop();
}

void SystemInfo::updateStats()
{
    readCpuStats();
    readMemoryStats();
    readDiskStats();
    readBatteryStats();
    emit uptimeChanged();
}

// CPU Usage - reads from /proc/stat
void SystemInfo::readCpuStats()
{
    QFile file(QStringLiteral("/proc/stat"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream stream(&file);
    QString line = stream.readLine();
    file.close();
    
    if (!line.startsWith(QLatin1String("cpu "))) {
        return;
    }
    
    // Parse: cpu user nice system idle iowait irq softirq steal guest guest_nice
    QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() < 5) {
        return;
    }
    
    qint64 user = parts[1].toLongLong();
    qint64 nice = parts[2].toLongLong();
    qint64 system = parts[3].toLongLong();
    qint64 idle = parts[4].toLongLong();
    qint64 iowait = parts.size() > 5 ? parts[5].toLongLong() : 0;
    qint64 irq = parts.size() > 6 ? parts[6].toLongLong() : 0;
    qint64 softirq = parts.size() > 7 ? parts[7].toLongLong() : 0;
    qint64 steal = parts.size() > 8 ? parts[8].toLongLong() : 0;
    
    qint64 totalIdle = idle + iowait;
    qint64 total = user + nice + system + totalIdle + irq + softirq + steal;
    
    qint64 totalDiff = total - m_prevCpuTotal;
    qint64 idleDiff = totalIdle - m_prevCpuIdle;
    
    if (totalDiff > 0) {
        double newUsage = 100.0 * (1.0 - (static_cast<double>(idleDiff) / static_cast<double>(totalDiff)));
        if (std::abs(newUsage - m_cpuUsage) > 0.1) {
            m_cpuUsage = newUsage;
            emit cpuUsageChanged();
        }
    }
    
    m_prevCpuTotal = total;
    m_prevCpuIdle = totalIdle;
}

// Memory - reads from /proc/meminfo
void SystemInfo::readMemoryStats()
{
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    
    QTextStream stream(&file);
    qint64 memTotal = 0;
    qint64 memAvailable = 0;
    bool foundTotal = false;
    bool foundAvailable = false;
    
    while (!stream.atEnd() && (!foundTotal || !foundAvailable)) {
        QString line = stream.readLine();
        
        if (line.startsWith(QLatin1String("MemTotal:"))) {
            QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                memTotal = parts[1].toLongLong() * 1024; // Convert KB to bytes
                foundTotal = true;
            }
        } else if (line.startsWith(QLatin1String("MemAvailable:"))) {
            QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                memAvailable = parts[1].toLongLong() * 1024;
                foundAvailable = true;
            }
        }
    }
    file.close();
    
    if (foundTotal && foundAvailable) {
        bool changed = (m_totalMemory != memTotal || m_availableMemory != memAvailable);
        m_totalMemory = memTotal;
        m_availableMemory = memAvailable;
        if (changed) {
            emit memoryChanged();
        }
    }
}

// Disk - uses QStorageInfo for root partition
void SystemInfo::readDiskStats()
{
    QStorageInfo storage = QStorageInfo::root();
    
    qint64 total = storage.bytesTotal();
    qint64 available = storage.bytesAvailable();
    
    bool changed = (m_totalDisk != total || m_availableDisk != available);
    m_totalDisk = total;
    m_availableDisk = available;
    
    if (changed) {
        emit diskChanged();
    }
}

// Battery - reads from /sys/class/power_supply
void SystemInfo::readBatteryStats()
{
    QDir powerSupply(QStringLiteral("/sys/class/power_supply"));
    if (!powerSupply.exists()) {
        m_hasBattery = false;
        return;
    }
    
    QStringList entries = powerSupply.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    
    for (const QString &entry : entries) {
        QString typePath = powerSupply.filePath(entry + QStringLiteral("/type"));
        QFile typeFile(typePath);
        if (!typeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        
        QString type = QString::fromUtf8(typeFile.readAll()).trimmed();
        typeFile.close();
        
        if (type != QLatin1String("Battery")) {
            continue;
        }
        
        m_hasBattery = true;
        
        // Read capacity
        QString capacityPath = powerSupply.filePath(entry + QStringLiteral("/capacity"));
        QFile capacityFile(capacityPath);
        if (capacityFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            int newPercent = QString::fromUtf8(capacityFile.readAll()).trimmed().toInt();
            capacityFile.close();
            
            // Read status
            QString statusPath = powerSupply.filePath(entry + QStringLiteral("/status"));
            QFile statusFile(statusPath);
            bool newCharging = false;
            if (statusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString status = QString::fromUtf8(statusFile.readAll()).trimmed();
                newCharging = (status == QLatin1String("Charging"));
                statusFile.close();
            }
            
            if (m_batteryPercent != newPercent || m_batteryCharging != newCharging) {
                m_batteryPercent = newPercent;
                m_batteryCharging = newCharging;
                emit batteryChanged();
            }
        }
        
        break; // Use first battery found
    }
}

// System info - static values read once
void SystemInfo::readSystemInfo()
{
    // Hostname
    QFile hostnameFile(QStringLiteral("/etc/hostname"));
    if (hostnameFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_hostname = QString::fromUtf8(hostnameFile.readAll()).trimmed();
        hostnameFile.close();
    } else {
        m_hostname = QStringLiteral("nakumios");
    }
    
    // Kernel version from /proc/version
    QFile versionFile(QStringLiteral("/proc/version"));
    if (versionFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString version = QString::fromUtf8(versionFile.readAll());
        QStringList parts = version.split(QLatin1Char(' '));
        if (parts.size() >= 3) {
            m_kernelVersion = parts[2];
        }
        versionFile.close();
    }
    
    // OS name
    m_osName = QStringLiteral("NakumiOS");
    QFile osReleaseFile(QStringLiteral("/etc/os-release"));
    if (osReleaseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&osReleaseFile);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith(QLatin1String("PRETTY_NAME="))) {
                m_osName = line.mid(13).remove(QLatin1Char('"'));
                break;
            }
        }
        osReleaseFile.close();
    }
}

// Getters
double SystemInfo::cpuUsage() const
{
    return m_cpuUsage;
}

qint64 SystemInfo::totalMemory() const
{
    return m_totalMemory;
}

qint64 SystemInfo::usedMemory() const
{
    return m_totalMemory - m_availableMemory;
}

qint64 SystemInfo::availableMemory() const
{
    return m_availableMemory;
}

double SystemInfo::memoryUsagePercent() const
{
    if (m_totalMemory == 0) return 0.0;
    return 100.0 * static_cast<double>(usedMemory()) / static_cast<double>(m_totalMemory);
}

qint64 SystemInfo::totalDisk() const
{
    return m_totalDisk;
}

qint64 SystemInfo::usedDisk() const
{
    return m_totalDisk - m_availableDisk;
}

qint64 SystemInfo::availableDisk() const
{
    return m_availableDisk;
}

double SystemInfo::diskUsagePercent() const
{
    if (m_totalDisk == 0) return 0.0;
    return 100.0 * static_cast<double>(usedDisk()) / static_cast<double>(m_totalDisk);
}

bool SystemInfo::hasBattery() const
{
    return m_hasBattery;
}

int SystemInfo::batteryPercent() const
{
    return m_batteryPercent;
}

bool SystemInfo::batteryCharging() const
{
    return m_batteryCharging;
}

QString SystemInfo::hostname() const
{
    return m_hostname;
}

QString SystemInfo::kernelVersion() const
{
    return m_kernelVersion;
}

QString SystemInfo::osName() const
{
    return m_osName;
}

qint64 SystemInfo::uptime() const
{
    QFile file(QStringLiteral("/proc/uptime"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }
    
    QString content = QString::fromUtf8(file.readAll());
    file.close();
    
    QStringList parts = content.split(QLatin1Char(' '));
    if (parts.isEmpty()) {
        return 0;
    }
    
    return static_cast<qint64>(parts[0].toDouble());
}

QString SystemInfo::formatBytes(qint64 bytes) const
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QStringLiteral("%1 %2").arg(size, 0, 'f', 1).arg(QLatin1String(units[unitIndex]));
}

QString SystemInfo::formatUptime(qint64 seconds) const
{
    qint64 days = seconds / 86400;
    qint64 hours = (seconds % 86400) / 3600;
    qint64 minutes = (seconds % 3600) / 60;
    
    if (days > 0) {
        return QStringLiteral("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
    } else if (hours > 0) {
        return QStringLiteral("%1h %2m").arg(hours).arg(minutes);
    } else {
        return QStringLiteral("%1m").arg(minutes);
    }
}

} // namespace NakumiOS
