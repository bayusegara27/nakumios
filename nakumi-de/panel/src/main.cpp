/*
 * nakumi-panel main entry point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "layershellwindow.h"
#include "panelcontroller.h"
#include "taskmanager.h"
#include "systemtray.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-panel"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    QQmlApplicationEngine engine;

    /* Register QML types */
    qmlRegisterType<PanelController>("NakumiOS.Panel", 1, 0, "PanelController");
    qmlRegisterType<TaskManager>("NakumiOS.Panel", 1, 0, "TaskManager");
    qmlRegisterType<SystemTray>("NakumiOS.Panel", 1, 0, "SystemTray");

    engine.load(QUrl(QStringLiteral("qrc:/qml/Panel.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    /* Attach layer-shell to the QML window */
    QQuickWindow *window =
        qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    if (window) {
        auto *layerShell = new LayerShellWindow(window, window);
        engine.rootContext()->setContextProperty(
            QStringLiteral("layerShell"), layerShell);
        layerShell->attachToLayer();
    }

    return app.exec();
}
