/*
 * nakumi-settings main entry point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "settingsmanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-settings"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    qmlRegisterType<SettingsManager>("NakumiOS.Settings", 1, 0, "SettingsManager");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/Settings.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
