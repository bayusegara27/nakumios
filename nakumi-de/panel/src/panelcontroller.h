/*
 * PanelController - QML-accessible controller for the NakumiOS panel
 *
 * Provides clock, date, and session management to QML.
 * Uses org.freedesktop.login1.Manager D-Bus interface for
 * shutdown/reboot to avoid spawning subprocesses.
 */

#ifndef PANELCONTROLLER_H
#define PANELCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QQmlEngine>

class PanelController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString currentTime READ currentTime NOTIFY timeChanged)
    Q_PROPERTY(QString currentDate READ currentDate NOTIFY timeChanged)

public:
    explicit PanelController(QObject *parent = nullptr);

    QString currentTime() const;
    QString currentDate() const;

    Q_INVOKABLE void launchApp(const QString &command);
    Q_INVOKABLE void toggleLauncher();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void reboot();

signals:
    void timeChanged();
    void launcherToggled();

private:
    QTimer m_clockTimer;
};

#endif // PANELCONTROLLER_H
