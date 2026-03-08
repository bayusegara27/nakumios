/**
 * NakumiOS Settings Application Main
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QIcon>

#include "settingscontroller.h"
#include "designtokens.h"
#include "systeminfo.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NakumiOS Settings"));
    app.setApplicationVersion(QStringLiteral(NAKUMIOS_VERSION));
    app.setOrganizationName(QStringLiteral("NakumiOS"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-system")));
    
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    
    // Register QML types
    qmlRegisterSingletonType<NakumiOS::DesignTokens>(
        "NakumiOS", 1, 0, "DesignTokens",
        NakumiOS::DesignTokens::create);
    
    qmlRegisterSingletonType<NakumiOS::SystemInfo>(
        "NakumiOS", 1, 0, "SystemInfo",
        NakumiOS::SystemInfo::create);
    
    qmlRegisterSingletonType<NakumiOS::SettingsController>(
        "NakumiOS.Settings", 1, 0, "SettingsController",
        NakumiOS::SettingsController::create);
    
    QQmlApplicationEngine engine;
    
    const QUrl mainUrl(QStringLiteral("qrc:/qml/Main.qml"));
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [mainUrl](QObject *obj, const QUrl &objUrl) {
        if (!obj && mainUrl == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(mainUrl);
    
    return app.exec();
}
