/**
 * NakumiOS Settings Controller
 * 
 * Backend for the settings application providing
 * system configuration management.
 */

#ifndef NAKUMIOS_SETTINGSCONTROLLER_H
#define NAKUMIOS_SETTINGSCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <QVariantMap>

namespace NakumiOS {

class SettingsController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    // Appearance settings
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged)
    Q_PROPERTY(qreal interfaceScale READ interfaceScale WRITE setInterfaceScale NOTIFY interfaceScaleChanged)
    
    // Display settings
    Q_PROPERTY(QString resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate NOTIFY refreshRateChanged)
    Q_PROPERTY(qreal brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(bool nightLight READ nightLight WRITE setNightLight NOTIFY nightLightChanged)
    
    // Sound settings
    Q_PROPERTY(int masterVolume READ masterVolume WRITE setMasterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY(bool muteAudio READ muteAudio WRITE setMuteAudio NOTIFY muteAudioChanged)

public:
    explicit SettingsController(QObject *parent = nullptr);
    ~SettingsController() override;
    
    static SettingsController *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    
    // Appearance
    QString accentColor() const;
    void setAccentColor(const QString &color);
    bool darkMode() const;
    void setDarkMode(bool enabled);
    qreal interfaceScale() const;
    void setInterfaceScale(qreal scale);
    
    // Display
    QString resolution() const;
    void setResolution(const QString &res);
    int refreshRate() const;
    void setRefreshRate(int rate);
    qreal brightness() const;
    void setBrightness(qreal level);
    bool nightLight() const;
    void setNightLight(bool enabled);
    
    // Sound
    int masterVolume() const;
    void setMasterVolume(int volume);
    bool muteAudio() const;
    void setMuteAudio(bool muted);
    
    Q_INVOKABLE QVariantList getAvailableResolutions() const;
    Q_INVOKABLE QVariantList getAvailableRefreshRates() const;
    Q_INVOKABLE void applySettings();
    Q_INVOKABLE void resetToDefaults();

signals:
    void accentColorChanged();
    void darkModeChanged();
    void interfaceScaleChanged();
    void resolutionChanged();
    void refreshRateChanged();
    void brightnessChanged();
    void nightLightChanged();
    void masterVolumeChanged();
    void muteAudioChanged();
    void settingsApplied();

private:
    void loadSettings();
    void saveSettings();
    
    QSettings *m_settings;
    
    QString m_accentColor;
    bool m_darkMode;
    qreal m_interfaceScale;
    QString m_resolution;
    int m_refreshRate;
    qreal m_brightness;
    bool m_nightLight;
    int m_masterVolume;
    bool m_muteAudio;
};

} // namespace NakumiOS

#endif // NAKUMIOS_SETTINGSCONTROLLER_H
