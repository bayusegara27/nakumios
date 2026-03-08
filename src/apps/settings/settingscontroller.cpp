/**
 * NakumiOS Settings Controller Implementation
 */

#include "settingscontroller.h"

#include <QFile>
#include <QDir>
#include <QScreen>
#include <QGuiApplication>

namespace NakumiOS {

SettingsController::SettingsController(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStringLiteral("NakumiOS"), QStringLiteral("Settings"), this))
    , m_accentColor(QStringLiteral("#6C5CE7"))
    , m_darkMode(true)
    , m_interfaceScale(1.0)
    , m_resolution(QStringLiteral("1920x1080"))
    , m_refreshRate(60)
    , m_brightness(1.0)
    , m_nightLight(false)
    , m_masterVolume(80)
    , m_muteAudio(false)
{
    loadSettings();
}

SettingsController::~SettingsController()
{
    saveSettings();
}

SettingsController *SettingsController::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(jsEngine)
    
    auto *controller = new SettingsController();
    QQmlEngine::setObjectOwnership(controller, QQmlEngine::JavaScriptOwnership);
    
    return controller;
}

// Appearance settings
QString SettingsController::accentColor() const
{
    return m_accentColor;
}

void SettingsController::setAccentColor(const QString &color)
{
    if (m_accentColor != color) {
        m_accentColor = color;
        saveSettings();
        emit accentColorChanged();
    }
}

bool SettingsController::darkMode() const
{
    return m_darkMode;
}

void SettingsController::setDarkMode(bool enabled)
{
    if (m_darkMode != enabled) {
        m_darkMode = enabled;
        saveSettings();
        emit darkModeChanged();
    }
}

qreal SettingsController::interfaceScale() const
{
    return m_interfaceScale;
}

void SettingsController::setInterfaceScale(qreal scale)
{
    if (m_interfaceScale != scale) {
        m_interfaceScale = qBound(0.5, scale, 3.0);
        saveSettings();
        emit interfaceScaleChanged();
    }
}

// Display settings
QString SettingsController::resolution() const
{
    return m_resolution;
}

void SettingsController::setResolution(const QString &res)
{
    if (m_resolution != res) {
        m_resolution = res;
        saveSettings();
        emit resolutionChanged();
    }
}

int SettingsController::refreshRate() const
{
    return m_refreshRate;
}

void SettingsController::setRefreshRate(int rate)
{
    if (m_refreshRate != rate) {
        m_refreshRate = rate;
        saveSettings();
        emit refreshRateChanged();
    }
}

qreal SettingsController::brightness() const
{
    return m_brightness;
}

void SettingsController::setBrightness(qreal level)
{
    if (m_brightness != level) {
        m_brightness = qBound(0.0, level, 1.0);
        
        // Apply brightness via sysfs if available
        QFile brightnessFile(QStringLiteral("/sys/class/backlight/intel_backlight/brightness"));
        QFile maxBrightnessFile(QStringLiteral("/sys/class/backlight/intel_backlight/max_brightness"));
        
        if (brightnessFile.exists() && maxBrightnessFile.exists()) {
            if (maxBrightnessFile.open(QIODevice::ReadOnly)) {
                int maxBrightness = QString::fromUtf8(maxBrightnessFile.readAll()).trimmed().toInt();
                maxBrightnessFile.close();
                
                int newBrightness = static_cast<int>(m_brightness * maxBrightness);
                
                if (brightnessFile.open(QIODevice::WriteOnly)) {
                    brightnessFile.write(QString::number(newBrightness).toUtf8());
                    brightnessFile.close();
                }
            }
        }
        
        saveSettings();
        emit brightnessChanged();
    }
}

bool SettingsController::nightLight() const
{
    return m_nightLight;
}

void SettingsController::setNightLight(bool enabled)
{
    if (m_nightLight != enabled) {
        m_nightLight = enabled;
        saveSettings();
        emit nightLightChanged();
    }
}

// Sound settings
int SettingsController::masterVolume() const
{
    return m_masterVolume;
}

void SettingsController::setMasterVolume(int volume)
{
    if (m_masterVolume != volume) {
        m_masterVolume = qBound(0, volume, 100);
        
        // Apply volume via pactl/amixer
        QString cmd = QStringLiteral("pactl set-sink-volume @DEFAULT_SINK@ %1%").arg(m_masterVolume);
        QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cmd});
        
        saveSettings();
        emit masterVolumeChanged();
    }
}

bool SettingsController::muteAudio() const
{
    return m_muteAudio;
}

void SettingsController::setMuteAudio(bool muted)
{
    if (m_muteAudio != muted) {
        m_muteAudio = muted;
        
        QString cmd = QStringLiteral("pactl set-sink-mute @DEFAULT_SINK@ %1").arg(muted ? "1" : "0");
        QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cmd});
        
        saveSettings();
        emit muteAudioChanged();
    }
}

QVariantList SettingsController::getAvailableResolutions() const
{
    QVariantList resolutions;
    
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        const QList<QSize> sizes = {
            QSize(3840, 2160),
            QSize(2560, 1440),
            QSize(1920, 1080),
            QSize(1600, 900),
            QSize(1366, 768),
            QSize(1280, 720)
        };
        
        for (const QSize &size : sizes) {
            resolutions.append(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
        }
    }
    
    return resolutions;
}

QVariantList SettingsController::getAvailableRefreshRates() const
{
    return QVariantList{30, 60, 75, 120, 144, 165, 240};
}

void SettingsController::applySettings()
{
    saveSettings();
    emit settingsApplied();
}

void SettingsController::resetToDefaults()
{
    m_accentColor = QStringLiteral("#6C5CE7");
    m_darkMode = true;
    m_interfaceScale = 1.0;
    m_resolution = QStringLiteral("1920x1080");
    m_refreshRate = 60;
    m_brightness = 1.0;
    m_nightLight = false;
    m_masterVolume = 80;
    m_muteAudio = false;
    
    saveSettings();
    
    emit accentColorChanged();
    emit darkModeChanged();
    emit interfaceScaleChanged();
    emit resolutionChanged();
    emit refreshRateChanged();
    emit brightnessChanged();
    emit nightLightChanged();
    emit masterVolumeChanged();
    emit muteAudioChanged();
}

void SettingsController::loadSettings()
{
    m_accentColor = m_settings->value(QStringLiteral("Appearance/AccentColor"), m_accentColor).toString();
    m_darkMode = m_settings->value(QStringLiteral("Appearance/DarkMode"), m_darkMode).toBool();
    m_interfaceScale = m_settings->value(QStringLiteral("Appearance/InterfaceScale"), m_interfaceScale).toReal();
    
    m_resolution = m_settings->value(QStringLiteral("Display/Resolution"), m_resolution).toString();
    m_refreshRate = m_settings->value(QStringLiteral("Display/RefreshRate"), m_refreshRate).toInt();
    m_brightness = m_settings->value(QStringLiteral("Display/Brightness"), m_brightness).toReal();
    m_nightLight = m_settings->value(QStringLiteral("Display/NightLight"), m_nightLight).toBool();
    
    m_masterVolume = m_settings->value(QStringLiteral("Sound/MasterVolume"), m_masterVolume).toInt();
    m_muteAudio = m_settings->value(QStringLiteral("Sound/MuteAudio"), m_muteAudio).toBool();
}

void SettingsController::saveSettings()
{
    m_settings->setValue(QStringLiteral("Appearance/AccentColor"), m_accentColor);
    m_settings->setValue(QStringLiteral("Appearance/DarkMode"), m_darkMode);
    m_settings->setValue(QStringLiteral("Appearance/InterfaceScale"), m_interfaceScale);
    
    m_settings->setValue(QStringLiteral("Display/Resolution"), m_resolution);
    m_settings->setValue(QStringLiteral("Display/RefreshRate"), m_refreshRate);
    m_settings->setValue(QStringLiteral("Display/Brightness"), m_brightness);
    m_settings->setValue(QStringLiteral("Display/NightLight"), m_nightLight);
    
    m_settings->setValue(QStringLiteral("Sound/MasterVolume"), m_masterVolume);
    m_settings->setValue(QStringLiteral("Sound/MuteAudio"), m_muteAudio);
    
    m_settings->sync();
}

} // namespace NakumiOS
