#include "physworldview_gl_quick.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#include <QRunnable>

#define SK_GANESH
#define SK_GL
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkColorType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"

sk_sp<const GrGLInterface> interface = nullptr;
GrDirectContext* skDirectContext = nullptr;
SkImageInfo info;
SkSurface* skSurface = nullptr;
SkCanvas* skCanvas = nullptr;

PhysicsWorldView::PhysicsWorldView(): m_renderer(nullptr)
{
    connect(this, &QQuickItem::windowChanged, this, &PhysicsWorldView::handleWindowChanged);
}
void PhysicsWorldView::handleWindowChanged(QQuickWindow* win)
{
    if (win) {
        connect(win, &QQuickWindow::beforeSynchronizing, this, &PhysicsWorldView::sync, Qt::DirectConnection);
        connect(win, &QQuickWindow::sceneGraphInvalidated, this, &PhysicsWorldView::cleanup, Qt::DirectConnection);
        win->setColor(QColor("lawngreen"));
    }
}
void PhysicsWorldView::cleanup()
{
    delete m_renderer;
    m_renderer = nullptr;
}
class CleanupJob : public QRunnable
{
public:
    CleanupJob(PhysWorldViewRenderer* renderer) : m_renderer(renderer) {}
    void run() override { delete m_renderer; }
private:
    PhysWorldViewRenderer* m_renderer;
};
void PhysicsWorldView::releaseResources()
{
    window()->scheduleRenderJob(new CleanupJob(m_renderer), QQuickWindow::BeforeSynchronizingStage);
    m_renderer = nullptr;
    delete skSurface;
    delete skDirectContext;
}

PhysWorldViewRenderer::~PhysWorldViewRenderer()
{
    //delete m_program;
}
//! [6]

//! [9]
void PhysicsWorldView::sync()
{
    if (!m_renderer) {
        m_renderer = new PhysWorldViewRenderer();
        connect(window(), &QQuickWindow::beforeRendering, m_renderer, &PhysWorldViewRenderer::init, Qt::DirectConnection);
        connect(window(), &QQuickWindow::beforeRenderPassRecording, m_renderer, &PhysWorldViewRenderer::paint, Qt::DirectConnection);
    }
    m_renderer->setViewportSize(window()->size() * window()->devicePixelRatio());
    m_renderer->setWindow(window());
}
//! [9]

//! [4]
void PhysWorldViewRenderer::init()
{
    QSGRendererInterface *rif = m_window->rendererInterface();
    Q_ASSERT(rif->graphicsApi() == QSGRendererInterface::OpenGL);
    initializeOpenGLFunctions();
    auto context = QOpenGLContext::currentContext();
    //QOpenGLContext::currentContext()->makeCurrent(m_window);
    Q_ASSERT(QOpenGLContext::currentContext());
    auto grGlInterface = GrGLMakeNativeInterface();
    if (grGlInterface == nullptr) {
        grGlInterface = GrGLMakeAssembledInterface(
            nullptr, (GrGLGetProc)*[](void*,const char* p) -> void* {
                return (void*)QOpenGLContext::currentContext()->getProcAddress(p); }
            );
    }
    Q_ASSERT(grGlInterface != nullptr);
    skDirectContext = GrDirectContexts::MakeGL(grGlInterface).release();
    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = 0;
    fbInfo.fFormat = GR_GL_RGBA8;
    SkColorType colorType = kRGBA_8888_SkColorType;
    GrBackendRenderTarget backendRenderTarget = GrBackendRenderTargets::MakeGL(
        m_viewportSize.width(),
        m_viewportSize.height(),
        0, // sample count
        0, // stencil bits
        fbInfo
    );
    skSurface = SkSurfaces::WrapBackendRenderTarget(
        skDirectContext,
        backendRenderTarget,
        kBottomLeft_GrSurfaceOrigin,
        colorType,
        nullptr, // color space
        nullptr // surface properties
                    ).release();
    Q_ASSERT(skSurface != nullptr);
    skCanvas = skSurface->getCanvas();
}

//! [4] //! [5]
void PhysWorldViewRenderer::paint()
{
    // Play nice with the RHI. Not strictly needed when the scenegraph uses
    // OpenGL directly.
    //m_window->beginExternalCommands();
    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    SkStringPrintf(__FUNCTION__);
    // glClearStencil(0);
    // glStencilMask(0xFF);
    //glClear(GL_COLOR_BUFFER_BIT);
    /*
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    skCanvas->drawPaint(paint);
    skCanvas->drawRect({50,50,552,552},paint);
    skDirectContext->flush();
*/
    //m_window->endExternalCommands();
}
//! [5]
