/**
 * NakumiOS File Manager Application Main
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QIcon>

#include "filemanagercontroller.h"
#include "designtokens.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NakumiOS Files"));
    app.setApplicationVersion(QStringLiteral(NAKUMIOS_VERSION));
    app.setOrganizationName(QStringLiteral("NakumiOS"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("system-file-manager")));
    
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    
    // Register QML types
    qmlRegisterSingletonType<NakumiOS::DesignTokens>(
        "NakumiOS", 1, 0, "DesignTokens",
        NakumiOS::DesignTokens::create);
    
    qmlRegisterSingletonType<NakumiOS::FileManagerController>(
        "NakumiOS.Files", 1, 0, "FileManager",
        NakumiOS::FileManagerController::create);
    
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
