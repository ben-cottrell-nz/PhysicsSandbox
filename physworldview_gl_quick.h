#ifndef PHYSWORLDVIEW_GL_QUICK_H
#define PHYSWORLDVIEW_GL_QUICK_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickWindow>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

class PhysWorldViewRenderer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    ~PhysWorldViewRenderer();
    void setViewportSize(const QSize &size) { m_viewportSize = size; }
    void setWindow(QQuickWindow *window) { m_window = window; }

public slots:
    void init();
    void paint();

private:
    QSize m_viewportSize;
    QQuickWindow *m_window = nullptr;
};

class PhysicsWorldView : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    PhysicsWorldView();

public slots:
    void sync();
    void cleanup();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    void releaseResources() override;

    PhysWorldViewRenderer *m_renderer;
};

#endif // PHYSWORLDVIEW_GL_QUICK_H
