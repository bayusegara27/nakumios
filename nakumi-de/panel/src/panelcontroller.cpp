/*
 * PanelController implementation
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
    /* Terminate the compositor gracefully */
    QProcess::startDetached(QStringLiteral("loginctl"),
                            QStringList{QStringLiteral("terminate-session"),
                                        QStringLiteral("self")});
}

void PanelController::shutdown() {
    QProcess::startDetached(QStringLiteral("systemctl"),
                            QStringList{QStringLiteral("poweroff")});
}

void PanelController::reboot() {
    QProcess::startDetached(QStringLiteral("systemctl"),
                            QStringList{QStringLiteral("reboot")});
}
