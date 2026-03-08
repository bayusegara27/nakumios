/*
 * SystemTray implementation
 */

#include "systemtray.h"

SystemTray::SystemTray(QObject *parent)
    : QObject(parent) {
    refreshVolume();
    refreshNetwork();
}

void SystemTray::setVolume(int vol) {
    vol = qBound(0, vol, 100);
    if (m_volume != vol) {
        m_volume = vol;
        /* Use wpctl to set volume via PipeWire/WirePlumber */
        QProcess::startDetached(
            QStringLiteral("wpctl"),
            QStringList{QStringLiteral("set-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@"),
                        QString::number(vol / 100.0)});
        emit volumeChanged();
    }
}

void SystemTray::refreshNetwork() {
    QProcess proc;
    proc.start(QStringLiteral("nmcli"), QStringList{
        QStringLiteral("-t"), QStringLiteral("-f"),
        QStringLiteral("ACTIVE,SSID"), QStringLiteral("dev"), QStringLiteral("wifi")});
    proc.waitForFinished(3000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();

    m_networkConnected = false;
    m_networkName.clear();

    const auto lines = output.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        if (line.startsWith(QLatin1String("yes:"))) {
            m_networkConnected = true;
            m_networkName = line.mid(4);
            break;
        }
    }
    emit networkChanged();
}

void SystemTray::refreshVolume() {
    QProcess proc;
    proc.start(QStringLiteral("wpctl"),
               QStringList{QStringLiteral("get-volume"),
                           QStringLiteral("@DEFAULT_AUDIO_SINK@")});
    proc.waitForFinished(3000);
    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    /* Output format: "Volume: 0.50" */
    if (output.contains(QLatin1String("Volume:"))) {
        QString volStr = output.split(QLatin1Char(':')).last().trimmed();
        bool ok = false;
        double vol = volStr.toDouble(&ok);
        if (ok) {
            m_volume = qBound(0, static_cast<int>(vol * 100), 100);
            emit volumeChanged();
        }
    }
}
