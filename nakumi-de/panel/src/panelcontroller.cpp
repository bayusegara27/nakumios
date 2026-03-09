/*
 * PanelController implementation
 *
 * Uses org.freedesktop.login1.Manager D-Bus interface for
 * system power management (shutdown, reboot) instead of spawning
 * systemctl subprocesses.
 */

#include "panelcontroller.h"

PanelController::PanelController(QObject *parent)
    : QObject(parent) {
    m_clockTimer.setInterval(1000);
    connect(&m_clockTimer, &QTimer::timeout, this, &PanelController::timeChanged);
    m_clockTimer.start();
}

QString PanelController::currentTime() const {
    return QDateTime::currentDateTime().toString(QStringLiteral("hh:mm"));
}

QString PanelController::currentDate() const {
    return QDateTime::currentDateTime().toString(QStringLiteral("ddd, MMM d"));
}

void PanelController::launchApp(const QString &command) {
    QProcess::startDetached(command, QStringList());
}

void PanelController::toggleLauncher() {
    emit launcherToggled();
    QProcess::startDetached(QStringLiteral("nakumi-launcher"), QStringList());
}

void PanelController::logout() {
    /* Terminate the compositor session via logind D-Bus */
    QDBusInterface logind(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus());

    if (logind.isValid()) {
        logind.asyncCall(QStringLiteral("TerminateSession"),
                         QStringLiteral("self"));
    }
}

void PanelController::shutdown() {
    /* Power off via org.freedesktop.login1.Manager.PowerOff(interactive) */
    QDBusInterface logind(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus());

    if (logind.isValid()) {
        logind.asyncCall(QStringLiteral("PowerOff"), true);
    }
}

void PanelController::reboot() {
    /* Reboot via org.freedesktop.login1.Manager.Reboot(interactive) */
    QDBusInterface logind(
        QStringLiteral("org.freedesktop.login1"),
        QStringLiteral("/org/freedesktop/login1"),
        QStringLiteral("org.freedesktop.login1.Manager"),
        QDBusConnection::systemBus());

    if (logind.isValid()) {
        logind.asyncCall(QStringLiteral("Reboot"), true);
    }
}
