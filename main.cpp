#include "mainwindow.h"
#include <QApplication>
#include <QOpenGLContext>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSurfaceFormat fmt;
    fmt.setSamples(24);
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
        fmt.setVersion(3,3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
    } else {
        fmt.setVersion(3,0);
    }
    QSurfaceFormat::setDefaultFormat(fmt);
    MainWindow w;
    w.resize(1152,968);
    w.show();
    return a.exec();
}
