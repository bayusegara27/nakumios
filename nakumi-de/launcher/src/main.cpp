/*
 * nakumi-launcher main entry point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "appmodel.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-launcher"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    qmlRegisterType<AppModel>("NakumiOS.Launcher", 1, 0, "AppModel");
    qmlRegisterType<AppFilterModel>("NakumiOS.Launcher", 1, 0, "AppFilterModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/Launcher.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
