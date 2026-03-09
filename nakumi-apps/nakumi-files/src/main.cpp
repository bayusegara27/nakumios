/*
 * nakumi-files main entry point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "filemanager.h"
#include "mountmanager.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-files"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    qmlRegisterType<FileManager>("NakumiOS.Files", 1, 0, "FileManager");
    qmlRegisterType<MountManager>("NakumiOS.Files", 1, 0, "MountManager");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/FileManager.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
