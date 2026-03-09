/*
 * nakumi-term main entry point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "terminalbackend.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-term"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    qmlRegisterType<TerminalBackend>("NakumiOS.Terminal", 1, 0, "TerminalBackend");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qml/Terminal.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
