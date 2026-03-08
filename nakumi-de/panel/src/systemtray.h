/*
 * SystemTray - Clock and system status for the panel
 */

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QObject>
#include <QProcess>
#include <QQmlEngine>

class SystemTray : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool networkConnected READ networkConnected NOTIFY networkChanged)
    Q_PROPERTY(QString networkName READ networkName NOTIFY networkChanged)

public:
    explicit SystemTray(QObject *parent = nullptr);

    int volume() const { return m_volume; }
    void setVolume(int vol);
    bool networkConnected() const { return m_networkConnected; }
    QString networkName() const { return m_networkName; }

    Q_INVOKABLE void refreshNetwork();
    Q_INVOKABLE void refreshVolume();

signals:
    void volumeChanged();
    void networkChanged();

private:
    int m_volume = 50;
    bool m_networkConnected = false;
    QString m_networkName;
};

#endif // SYSTEMTRAY_H
