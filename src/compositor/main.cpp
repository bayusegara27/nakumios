/**
 * NakumiOS Compositor Main Entry Point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QUrl>

#include "compositor.h"
#include "windowmanager.h"
#include "designtokens.h"
#include "systeminfo.h"

int main(int argc, char *argv[])
{
    // Set environment for Wayland compositor
    qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", "linux-dmabuf-unstable-v1");
    
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NakumiOS Compositor"));
    app.setApplicationVersion(QStringLiteral(NAKUMIOS_VERSION));
    app.setOrganizationName(QStringLiteral("NakumiOS"));
    
    // Register QML types
    qmlRegisterSingletonType<NakumiOS::DesignTokens>(
        "NakumiOS", 1, 0, "DesignTokens",
        NakumiOS::DesignTokens::create);
    
    qmlRegisterSingletonType<NakumiOS::SystemInfo>(
        "NakumiOS", 1, 0, "SystemInfo",
        NakumiOS::SystemInfo::create);
    
    qmlRegisterType<NakumiOS::Compositor>("NakumiOS.Compositor", 1, 0, "Compositor");
    qmlRegisterType<NakumiOS::WindowManager>("NakumiOS.Compositor", 1, 0, "WindowManager");
    
    QQmlApplicationEngine engine;
    
    // Load the main compositor QML
    const QUrl mainUrl(QStringLiteral("qrc:/qml/CompositorWindow.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [mainUrl](QObject *obj, const QUrl &objUrl) {
        if (!obj && mainUrl == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(mainUrl);
    
    return app.exec();
}
