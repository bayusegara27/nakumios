/*
 * SettingsManager implementation
 */

#include "settingsmanager.h"

#include <QDir>
#include <QFile>
#include <QSysInfo>

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent),
      m_settings(QDir::homePath() + QStringLiteral("/.config/nakumi/settings.ini"),
                 QSettings::IniFormat) {
    loadSettings();
    refreshAll();
}

void SettingsManager::loadSettings() {
    m_wallpaper = m_settings.value(QStringLiteral("appearance/wallpaper"),
                                   QStringLiteral("/usr/share/backgrounds/nakumi-default.png"))
                      .toString();
    m_accentColor = m_settings.value(QStringLiteral("appearance/accentColor"),
                                     QStringLiteral("#6C5CE7"))
                        .toString();
}

void SettingsManager::saveSettings() {
    m_settings.setValue(QStringLiteral("appearance/wallpaper"), m_wallpaper);
    m_settings.setValue(QStringLiteral("appearance/accentColor"), m_accentColor);
    m_settings.sync();
}

/* ======================================================================
 * Audio (wpctl)
 * ====================================================================== */

void SettingsManager::setVolume(int vol) {
    vol = qBound(0, vol, 100);
    if (m_volume != vol) {
        m_volume = vol;
        QProcess::startDetached(
            QStringLiteral("wpctl"),
            QStringList{QStringLiteral("set-volume"),
                        QStringLiteral("@DEFAULT_AUDIO_SINK@"),
                        QString::number(vol / 100.0)});
        emit volumeChanged();
    }
}

void SettingsManager::refreshVolume() {
    QProcess proc;
    proc.start(QStringLiteral("wpctl"),
               QStringList{QStringLiteral("get-volume"),
                           QStringLiteral("@DEFAULT_AUDIO_SINK@")});
    proc.waitForFinished(3000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    if (output.contains(QLatin1String("Volume:"))) {
        bool ok = false;
        double vol = output.split(QLatin1Char(':')).last().trimmed().toDouble(&ok);
        if (ok) {
            m_volume = qBound(0, static_cast<int>(vol * 100), 100);
            emit volumeChanged();
        }
    }
}

/* ======================================================================
 * Wi-Fi (nmcli)
 * ====================================================================== */

void SettingsManager::setWifiEnabled(bool enabled) {
    if (m_wifiEnabled != enabled) {
        m_wifiEnabled = enabled;
        QProcess::startDetached(
            QStringLiteral("nmcli"),
            QStringList{QStringLiteral("radio"), QStringLiteral("wifi"),
                        enabled ? QStringLiteral("on") : QStringLiteral("off")});
        emit wifiChanged();
    }
}

void SettingsManager::scanNetworks() {
    m_availableNetworks.clear();

    QProcess proc;
    proc.start(QStringLiteral("nmcli"),
               QStringList{QStringLiteral("-t"), QStringLiteral("-f"),
                           QStringLiteral("SSID"), QStringLiteral("dev"),
                           QStringLiteral("wifi"), QStringLiteral("list")});
    proc.waitForFinished(5000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();

    const auto lines = output.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        QString ssid = line.trimmed();
        if (!ssid.isEmpty() && !m_availableNetworks.contains(ssid)) {
            m_availableNetworks.append(ssid);
        }
    }
    emit wifiChanged();
}

void SettingsManager::connectToNetwork(const QString &ssid,
                                       const QString &password) {
    QProcess proc;
    QStringList args = {QStringLiteral("dev"), QStringLiteral("wifi"),
                        QStringLiteral("connect"), ssid};
    if (!password.isEmpty()) {
        args << QStringLiteral("password") << password;
    }
    proc.start(QStringLiteral("nmcli"), args);
    proc.waitForFinished(15000);

    if (proc.exitCode() == 0) {
        m_currentNetwork = ssid;
        emit wifiChanged();
    } else {
        emit errorOccurred(QStringLiteral("Failed to connect to ") + ssid);
    }
}

void SettingsManager::disconnectNetwork() {
    QProcess::startDetached(
        QStringLiteral("nmcli"),
        QStringList{QStringLiteral("dev"), QStringLiteral("disconnect"),
                    QStringLiteral("wlan0")});
    m_currentNetwork.clear();
    emit wifiChanged();
}

void SettingsManager::refreshWifi() {
    QProcess proc;
    proc.start(QStringLiteral("nmcli"),
               QStringList{QStringLiteral("-t"), QStringLiteral("-f"),
                           QStringLiteral("ACTIVE,SSID"), QStringLiteral("dev"),
                           QStringLiteral("wifi")});
    proc.waitForFinished(3000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();

    m_currentNetwork.clear();
    const auto lines = output.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        if (line.startsWith(QLatin1String("yes:"))) {
            m_currentNetwork = line.mid(4);
            break;
        }
    }
    emit wifiChanged();
}

/* ======================================================================
 * Appearance
 * ====================================================================== */

void SettingsManager::setWallpaper(const QString &path) {
    if (m_wallpaper != path) {
        m_wallpaper = path;
        saveSettings();
        emit wallpaperChanged();
    }
}

void SettingsManager::setAccentColor(const QString &color) {
    if (m_accentColor != color) {
        m_accentColor = color;
        saveSettings();
        emit themeChanged();
    }
}

/* ======================================================================
 * System Info
 * ====================================================================== */

QString SettingsManager::hostname() const {
    return QSysInfo::machineHostName();
}

QString SettingsManager::kernelVersion() const {
    return QSysInfo::kernelVersion();
}

QString SettingsManager::cpuInfo() const {
    QFile file(QStringLiteral("/proc/cpuinfo"));
    if (!file.open(QIODevice::ReadOnly)) return QStringLiteral("Unknown");
    QString content = QString::fromUtf8(file.readAll());
    for (const auto &line : content.split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1String("model name"))) {
            return line.section(QLatin1Char(':'), 1).trimmed();
        }
    }
    return QStringLiteral("Unknown");
}

QString SettingsManager::memoryInfo() const {
    QFile file(QStringLiteral("/proc/meminfo"));
    if (!file.open(QIODevice::ReadOnly)) return QStringLiteral("Unknown");
    QString content = QString::fromUtf8(file.readAll());
    for (const auto &line : content.split(QLatin1Char('\n'))) {
        if (line.startsWith(QLatin1String("MemTotal"))) {
            QString kbStr = line.section(QLatin1Char(':'), 1).trimmed();
            kbStr = kbStr.section(QLatin1Char(' '), 0, 0);
            bool ok = false;
            qint64 kb = kbStr.toLongLong(&ok);
            if (ok) {
                return QString::number(kb / 1024 / 1024.0, 'f', 1) +
                       QStringLiteral(" GB");
            }
        }
    }
    return QStringLiteral("Unknown");
}

void SettingsManager::refreshAll() {
    refreshVolume();
    refreshWifi();
    scanNetworks();
    emit systemInfoChanged();
}
