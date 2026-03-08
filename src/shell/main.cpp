/**
 * NakumiOS Shell Main Entry Point
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QFont>
#include <QFontDatabase>

#include "shellcontroller.h"
#include "designtokens.h"
#include "systeminfo.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NakumiOS Shell"));
    app.setApplicationVersion(QStringLiteral(NAKUMIOS_VERSION));
    app.setOrganizationName(QStringLiteral("NakumiOS"));
    
    // Set up font
    int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Inter-Regular.ttf"));
    if (fontId >= 0) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QFont defaultFont(fontFamilies.first());
            defaultFont.setPointSize(13);
            app.setFont(defaultFont);
        }
    }
    
    // Set Qt Quick style
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    
    // Register QML types
    qmlRegisterSingletonType<NakumiOS::DesignTokens>(
        "NakumiOS", 1, 0, "DesignTokens",
        NakumiOS::DesignTokens::create);
    
    qmlRegisterSingletonType<NakumiOS::SystemInfo>(
        "NakumiOS", 1, 0, "SystemInfo",
        NakumiOS::SystemInfo::create);
    
    qmlRegisterSingletonType<NakumiOS::ShellController>(
        "NakumiOS.Shell", 1, 0, "ShellController",
        NakumiOS::ShellController::create);
    
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
