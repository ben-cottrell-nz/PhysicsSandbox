#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickView>
#include <QSGRendererInterface>
#include "physworldview_skia_quickitem.h"

inline bool initializeSceneGraph()
{
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    if (QQuickWindow::graphicsApi() != QSGRendererInterface::OpenGL) {
        qDebug("Quick graphics API is not using OpenGL, exiting");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    //if (!initializeSceneGraph()) return 1;
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickView view;
    qmlRegisterType<PhysWorldViewSkia>("Extras", 1,0, "PhysWorldView");
    view.setSource(QUrl("qrc:/main.qml"));
    if (view.status() == QQuickView::Error)
        return -1;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    view.setWindowState(Qt::WindowMaximized);
    return app.exec();
}
