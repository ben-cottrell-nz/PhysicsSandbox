#include "mainwindow.h"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickView>
#include "physworldview_gl_quick.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    //QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QQuickView view;
    //view.loadFromModule(u"Extras", u"Main");
    qmlRegisterType<PhysicsWorldView>("Extras", 1, 0, "PhysicsWorldView");
    view.setSource(QUrl("qrc:/main.qml"));
    if (view.status() == QQuickView::Error)
        return -1;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}
