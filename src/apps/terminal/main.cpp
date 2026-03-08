/**
 * NakumiOS Terminal Application Main
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QUrl>
#include <QIcon>

#include "terminalcontroller.h"
#include "designtokens.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NakumiOS Terminal"));
    app.setApplicationVersion(QStringLiteral(NAKUMIOS_VERSION));
    app.setOrganizationName(QStringLiteral("NakumiOS"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("utilities-terminal")));
    
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    
    // Register QML types
    qmlRegisterSingletonType<NakumiOS::DesignTokens>(
        "NakumiOS", 1, 0, "DesignTokens",
        NakumiOS::DesignTokens::create);
    
    qmlRegisterType<NakumiOS::TerminalController>(
        "NakumiOS.Terminal", 1, 0, "TerminalController");
    
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
