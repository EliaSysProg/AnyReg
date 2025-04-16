#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "AnyRegBridge.hpp"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    RegBridge regBridge;
    engine.rootContext()->setContextProperty("regBridge", &regBridge);

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/anyregapp/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
