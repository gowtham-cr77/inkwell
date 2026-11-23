#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QDir>

#include "canvas/InkCanvas.h"
#include "storage/Database.h"
#include "library/NotebookManager.h"
#include "library/LibraryModel.h"
#include "ui/ThemeManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("Inkwell");
    app.setOrganizationName("Inkwell");
    app.setApplicationVersion("0.1.0");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/resources/icons/inkwell.svg"));
    
    // Set Qt Quick style
    QQuickStyle::setStyle("Basic");
    
    // Initialize database
    Inkwell::Database db;
    if (!db.initialize()) {
        qCritical() << "Failed to initialize database";
        return 1;
    }
    
    // Create managers
    Inkwell::NotebookManager notebookManager(&db);
    Inkwell::LibraryModel libraryModel(&notebookManager);
    Inkwell::ThemeManager themeManager;
    
    // Register QML types
    qmlRegisterType<Inkwell::InkCanvas>("Inkwell", 1, 0, "InkCanvas");
    
    QQmlApplicationEngine engine;
    
    // Expose singletons to QML
    engine.rootContext()->setContextProperty("notebookManager", &notebookManager);
    engine.rootContext()->setContextProperty("libraryModel", &libraryModel);
    engine.rootContext()->setContextProperty("themeManager", &themeManager);
    
    // Load main QML
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);
    
    if (engine.rootObjects().isEmpty())
        return -1;
    
    return app.exec();
}
