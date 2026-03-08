/*
 * nakumi-edit main entry point
 */

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QStatusBar>
#include <QVBoxLayout>

#include "editor.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("nakumi-edit"));
    app.setOrganizationName(QStringLiteral("NakumiOS"));

    /* Apply dark theme */
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(0x1A, 0x1A, 0x24));
    darkPalette.setColor(QPalette::WindowText, QColor(0xE2, 0xE8, 0xF0));
    darkPalette.setColor(QPalette::Base, QColor(0x1A, 0x1A, 0x24));
    darkPalette.setColor(QPalette::AlternateBase, QColor(0x2D, 0x2D, 0x3F));
    darkPalette.setColor(QPalette::Text, QColor(0xE2, 0xE8, 0xF0));
    darkPalette.setColor(QPalette::Button, QColor(0x2D, 0x2D, 0x3F));
    darkPalette.setColor(QPalette::ButtonText, QColor(0xE2, 0xE8, 0xF0));
    darkPalette.setColor(QPalette::Highlight, QColor(0x6C, 0x5C, 0xE7));
    darkPalette.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));
    app.setPalette(darkPalette);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("NakumiEdit"));
    window.resize(800, 600);

    auto *editor = new Editor(&window);
    window.setCentralWidget(editor);

    /* Menu bar */
    auto *fileMenu = window.menuBar()->addMenu(QStringLiteral("&File"));

    fileMenu->addAction(QStringLiteral("&Open..."), &window, [&]() {
        QString path = QFileDialog::getOpenFileName(&window, QStringLiteral("Open File"));
        if (!path.isEmpty()) {
            editor->openFile(path);
            window.setWindowTitle(QStringLiteral("NakumiEdit - ") + path);
        }
    }, QKeySequence::Open);

    fileMenu->addAction(QStringLiteral("&Save"), &window, [&]() {
        if (editor->currentFile().isEmpty()) {
            QString path = QFileDialog::getSaveFileName(&window, QStringLiteral("Save File"));
            if (!path.isEmpty()) {
                editor->saveFileAs(path);
                window.setWindowTitle(QStringLiteral("NakumiEdit - ") + path);
            }
        } else {
            editor->saveFile();
        }
    }, QKeySequence::Save);

    fileMenu->addAction(QStringLiteral("Save &As..."), &window, [&]() {
        QString path = QFileDialog::getSaveFileName(&window, QStringLiteral("Save File As"));
        if (!path.isEmpty()) {
            editor->saveFileAs(path);
            window.setWindowTitle(QStringLiteral("NakumiEdit - ") + path);
        }
    }, QKeySequence::SaveAs);

    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("&Quit"), &app, &QApplication::quit, QKeySequence::Quit);

    /* Status bar */
    QObject::connect(editor, &Editor::statusMessage,
                     window.statusBar(), &QStatusBar::showMessage);

    /* Open file from command line argument */
    if (argc > 1) {
        editor->openFile(QString::fromLocal8Bit(argv[1]));
        window.setWindowTitle(QStringLiteral("NakumiEdit - ") + argv[1]);
    }

    window.show();
    return app.exec();
}
