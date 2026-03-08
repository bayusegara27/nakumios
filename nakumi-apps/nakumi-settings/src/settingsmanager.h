/*
 * SettingsManager - System configuration backend for NakumiSettings
 *
 * Uses QProcess for Wi-Fi (nmcli) and Audio (wpctl).
 * Reads/writes configs from ~/.config/nakumi/.
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QSettings>
#include <QQmlEngine>

class SettingsManager : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool wifiEnabled READ wifiEnabled WRITE setWifiEnabled NOTIFY wifiChanged)
    Q_PROPERTY(QString currentNetwork READ currentNetwork NOTIFY wifiChanged)
    Q_PROPERTY(QStringList availableNetworks READ availableNetworks NOTIFY wifiChanged)
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY wallpaperChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY themeChanged)
    Q_PROPERTY(QString hostname READ hostname NOTIFY systemInfoChanged)
    Q_PROPERTY(QString kernelVersion READ kernelVersion NOTIFY systemInfoChanged)
    Q_PROPERTY(QString cpuInfo READ cpuInfo NOTIFY systemInfoChanged)
    Q_PROPERTY(QString memoryInfo READ memoryInfo NOTIFY systemInfoChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    /* Audio */
    int volume() const { return m_volume; }
    void setVolume(int vol);

    /* Wi-Fi */
    bool wifiEnabled() const { return m_wifiEnabled; }
    void setWifiEnabled(bool enabled);
    QString currentNetwork() const { return m_currentNetwork; }
    QStringList availableNetworks() const { return m_availableNetworks; }
    Q_INVOKABLE void connectToNetwork(const QString &ssid, const QString &password);
    Q_INVOKABLE void disconnectNetwork();
    Q_INVOKABLE void scanNetworks();

    /* Appearance */
    QString wallpaper() const { return m_wallpaper; }
    void setWallpaper(const QString &path);
    QString accentColor() const { return m_accentColor; }
    void setAccentColor(const QString &color);

    /* System info */
    QString hostname() const;
    QString kernelVersion() const;
    QString cpuInfo() const;
    QString memoryInfo() const;

    Q_INVOKABLE void refreshAll();

signals:
    void volumeChanged();
    void wifiChanged();
    void wallpaperChanged();
    void themeChanged();
    void systemInfoChanged();
    void errorOccurred(const QString &message);

private:
    void loadSettings();
    void saveSettings();
    void refreshVolume();
    void refreshWifi();

    QSettings m_settings;
    int m_volume = 50;
    bool m_wifiEnabled = true;
    QString m_currentNetwork;
    QStringList m_availableNetworks;
    QString m_wallpaper;
    QString m_accentColor = QStringLiteral("#6C5CE7");
};

#endif // SETTINGSMANAGER_H
