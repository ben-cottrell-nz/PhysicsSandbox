// Based on https://github.com/QtSkia/QtSkia/blob/master/QtSkia/QtSkiaQuick/QuickItem/QSkiaQuickItem.cpp
#include "SkiaQuickItem.h"
#include "include/gpu/ganesh/GrBackendSurface.h"

#define SK_GANESH
#define SK_GL
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"

#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include <QtGlobal>

#include <QMutex>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QThread>
#include <QSGTexture>

class TextureNode : public QObject, public QSGSimpleTextureNode {
    Q_OBJECT
public:
    TextureNode(QQuickWindow* window)
        : m_window(window)
    {
        //m_texture = m_window->createTextureFromId(0, QSize(1, 1));
        m_texture = QNativeInterface::QSGOpenGLTexture::fromNative(0, m_window, QSize(1,1));
        setTexture(m_texture);
        setFiltering(QSGTexture::Linear);

        setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    }
    ~TextureNode() override
    {
        delete m_texture;
    }
signals:
    void textureInUse();
    void pendingNewTexture();
public slots:
    void newTexture(uint id, const QSize& size)
    {
        m_mutex.lock();
        m_id = id;
        m_size = size;
        m_mutex.unlock();

        emit pendingNewTexture();
    }
    void prepareNode()
    {
        m_mutex.lock();
        uint newid = m_id;
        QSize size = m_size;
        m_id = 0;
        m_mutex.unlock();
        if (newid) {
            delete m_texture;
            //m_texture = m_window->createTextureFromId(newid, size);
            m_texture = QNativeInterface::QSGOpenGLTexture::fromNative(newid, m_window, size);
            setTexture(m_texture);
            markDirty(DirtyMaterial);

            emit textureInUse();
        }
    }

private:
    QMutex m_mutex;
    uint m_id = 0;
    QSize m_size = { 0, 0 };

    QSGTexture* m_texture = nullptr;
    QQuickWindow* m_window;
};

class SkiaObj : public QObject {
    Q_OBJECT
public:
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;

public:
    SkiaObj(const QSize& size, SkiaQuickItem* item)
        : m_item(item)
        , m_size(size)
        , m_newWidth(size.width())
        , m_newHeight(size.height())
    {
    }
    void initSizeBeforeReady() {
        m_size = m_item->size().toSize();
        m_newWidth = m_size.width();
        m_newHeight = m_size.height();
    }
signals:
    void textureReady(uint id, const QSize& size);
    void over();
public slots:
    void renderNext()
    {
        context->makeCurrent(surface);
        if (!m_renderFbo) {
            createFBO();
            swapped = false;
            m_item->onInit(m_size.width(), m_size.height());
        } else if (needResize) {
            delete m_renderFbo;
            delete m_displayFbo;
            m_renderFbo = nullptr;
            m_displayFbo = nullptr;
            m_size = QSize(m_newWidth, m_newHeight);
            createFBO();
            needResize = false;
            m_item->onResize(m_size.width(), m_size.height());
        }
        if (m_renderFbo != nullptr)
            m_renderFbo->bind();
        context->functions()->glViewport(0, 0, m_size.width(), m_size.height());

        //render
        if (swapped) {
            displaySurface->getCanvas()->save();
            m_item->draw(displaySurface->getCanvas(), 16);
        } else {
            Q_ASSERT(renderSurface.get() != nullptr);
            Q_ASSERT(renderSurface->getCanvas() != nullptr);
            renderSurface->getCanvas()->save();
            SkAutoCanvasRestore cs(renderSurface->getCanvas(), true);
            m_item->draw(renderSurface->getCanvas(), 16);
        }
        skiaContext->flush();
        context->functions()->glFlush();
        m_renderFbo->bindDefault();
        std::swap(m_renderFbo, m_displayFbo);
        swapped = !swapped;
        emit textureReady(m_displayFbo->texture(), m_size);
        if (!swapped) {
            displaySurface->getCanvas()->restore();
        } else {
            renderSurface->getCanvas()->restore();
        }
    }
    void shutdown()
    {
        context->makeCurrent(surface);
        //delete m_renderFbo;
        //delete m_displayFbo;
        //free skia
        skiaContext = nullptr;
        renderSurface = nullptr;
        displaySurface = nullptr;
        context->doneCurrent();
        delete context;
        surface->deleteLater();
        emit over();
    }
    void syncNewSize() {
        auto size = m_item->size().toSize();
        if (m_newWidth != size.width()) {
            m_newWidth = size.width();
            needResize = true;
        }
        if (m_newHeight != size.height()) {
            m_newHeight = size.height();
            needResize = true;
        }
    }
protected:
    void createFBO() {
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        m_renderFbo = new QOpenGLFramebufferObject(m_size, format);
        m_displayFbo = new QOpenGLFramebufferObject(m_size, format);
        //init skia
        auto interface = GrGLMakeNativeInterface();
        if (interface == nullptr) {
            //backup plan. see https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add?permalink_comment_id=4680136#gistcomment-4680136
            interface = GrGLMakeAssembledInterface(
                nullptr, (GrGLGetProc) * [](void*, const char* p) -> void* {
                    return (void*)QOpenGLContext::currentContext()->getProcAddress(p);
                }
                );
        }
        skiaContext = GrDirectContexts::MakeGL(interface);
        SkColorType colorType;
        colorType = kRGBA_8888_SkColorType;
        // setup SkSurface
        // To use distance field text, use commented out SkSurfaceProps instead
        SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,SkPixelGeometry::kUnknown_SkPixelGeometry);
        //    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

        {
            GrGLFramebufferInfo info;
            info.fFBOID = m_renderFbo->handle();
            info.fFormat = GR_GL_RGBA8;

            //GrBackendRenderTarget backend(m_size.width(), m_size.height(), format.samples(), QSurfaceFormat::defaultFormat().stencilBufferSize(), info);

            // GrBackendRenderTarget backend(m_size.width(),m_size.height(),
            //                               0/* msaa samples */,8 /* stencil bits */,
            //                               GrGLFramebufferInfo{m_renderFbo->handle(), GL_RGBA8});
            GrBackendRenderTarget backend = GrBackendRenderTargets::MakeGL(
                m_size.width(),m_size.height(), 0, 8, info);
            /*
            renderSurface = SkSurfaces::MakeFromBackendRenderTarget(
                skiaContext.get(), backend, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props);
            */
            renderSurface = SkSurfaces::WrapBackendRenderTarget(
                skiaContext.get(), backend, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props);
            Q_ASSERT(renderSurface != nullptr);
        }
        {
            GrGLFramebufferInfo info;
            info.fFBOID = m_displayFbo->handle();
            info.fFormat = GR_GL_RGBA8;

            //GrBackendRenderTarget backend { m_size.width(), m_size.height(), format.samples(), (int)QSurfaceFormat::defaultFormat().stencilBufferSize(), info};
            // GrBackendRenderTarget backend(m_size.width(),m_size.height(),
            //                               0/* msaa samples */,8 /* stencil bits */,
            //                               GrGLFramebufferInfo{m_renderFbo->handle(), GL_RGBA8});
            GrBackendRenderTarget backend = GrBackendRenderTargets::MakeGL(
                m_size.width(),m_size.height(), 0, 8, info);
            //displaySurface = SkSurface::MakeFromBackendRenderTarget(skiaContext.get(), backend, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props);
            displaySurface = SkSurfaces::WrapBackendRenderTarget(
                skiaContext.get(), backend, kBottomLeft_GrSurfaceOrigin, colorType, nullptr, &props);
            Q_ASSERT(displaySurface != nullptr);
        }
    }
private:
    std::atomic<bool> needResize = false;
    bool swapped = false;
    SkiaQuickItem* m_item;
    QOpenGLFramebufferObject* m_renderFbo = nullptr;
    QOpenGLFramebufferObject* m_displayFbo = nullptr;
    sk_sp<GrDirectContext> skiaContext = nullptr;
    sk_sp<SkSurface> renderSurface = nullptr;
    sk_sp<SkSurface> displaySurface = nullptr;
    QSize m_size;
    QAtomicInt m_newWidth;
    QAtomicInt m_newHeight;
};
class SkiaQuickItemPrivate : public QObject{
    Q_OBJECT
public:
    SkiaQuickItemPrivate(SkiaQuickItem* pItem)
        : item(pItem)
    {
        thread = new QThread;
        skiaObj = new SkiaObj(QSize{512, 512}, item);
    }
    void init() //in QSGRender Thread
    {
        //QOpenGLContext* current = item->window()->openglContext();
        QOpenGLContext* current = reinterpret_cast<QOpenGLContext*>(item->window()->rendererInterface()->getResource(item->window(), QSGRendererInterface::OpenGLContextResource));

        current->doneCurrent();
        skiaObj->context = new QOpenGLContext;
        auto format = current->format();
        skiaObj->context->setFormat(format);
        skiaObj->context->setShareContext(current);
        skiaObj->context->create();
        skiaObj->context->moveToThread(thread);

        current->makeCurrent(item->window());
        inited = true;
    }
public slots:
    void ready() //queue call in item thread
    {
        skiaObj->surface = new QOffscreenSurface;
        skiaObj->surface->setFormat(skiaObj->context->format());
        skiaObj->surface->create();

        skiaObj->initSizeBeforeReady();

        skiaObj->moveToThread(thread);
        QObject::connect(item->window(), &QQuickWindow::sceneGraphInvalidated, skiaObj, &SkiaObj::shutdown, Qt::QueuedConnection);
        QObject::connect(skiaObj, &SkiaObj::over, thread, &QThread::quit, Qt::QueuedConnection);
        QObject::connect(skiaObj, &SkiaObj::over, skiaObj, &QObject::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
        item->update();
    }

public:
    bool inited = false;
    SkiaQuickItem* item;
    QThread *thread;
    SkiaObj *skiaObj;
};
SkiaQuickItem::SkiaQuickItem(QQuickItem* parent)
    : QQuickItem(parent)
    , m_dptr(new SkiaQuickItemPrivate(this))
{
    setFlag(ItemHasContents, true);
}

SkiaQuickItem::~SkiaQuickItem()
{
    delete m_dptr;
}

QSGNode* SkiaQuickItem::updatePaintNode(QSGNode* oldNode, QQuickItem::UpdatePaintNodeData*)
{
    TextureNode* node = static_cast<TextureNode*>(oldNode);
    if (width() <= 0 || height() <= 0) {
        return nullptr;
    }
    if (!m_dptr->inited) {
        m_dptr->init();
        QMetaObject::invokeMethod(m_dptr, "ready");
        return nullptr;
    }

    if (!node) {
        node = new TextureNode(window());
        /* Set up connections to get the production of FBO textures in sync with vsync on the
         * rendering thread.
         *
         * When a new texture is ready on the rendering thread, we use a direct connection to
         * the texture node to let it know a new texture can be used. The node will then
         * emit pendingNewTexture which we bind to QQuickWindow::update to schedule a redraw.
         *
         * When the scene graph starts rendering the next frame, the prepareNode() function
         * is used to update the node with the new texture. Once it completes, it emits
         * textureInUse() which we connect to the FBO rendering thread's renderNext() to have
         * it start producing content into its current "back buffer".
         *
         * This FBO rendering pipeline is throttled by vsync on the scene graph rendering thread.
         */
        connect(m_dptr->skiaObj, &SkiaObj::textureReady, node, &TextureNode::newTexture, Qt::DirectConnection);
        connect(node, &TextureNode::pendingNewTexture, window(), &QQuickWindow::update, Qt::QueuedConnection);
        connect(window(), &QQuickWindow::beforeRendering, node, &TextureNode::prepareNode, Qt::DirectConnection);
        connect(node, &TextureNode::textureInUse, m_dptr->skiaObj, &SkiaObj::renderNext, Qt::QueuedConnection);

        connect(this, &QQuickItem::widthChanged, m_dptr->skiaObj, &SkiaObj::syncNewSize, Qt::QueuedConnection);
        connect(this, &QQuickItem::heightChanged, m_dptr->skiaObj, &SkiaObj::syncNewSize, Qt::QueuedConnection);
        // Get the production of FBO textures started..
        QMetaObject::invokeMethod(m_dptr->skiaObj, "renderNext", Qt::QueuedConnection);
    }
    node->setRect(boundingRect());
    return node;
}

#include "SkiaQuickItem.moc"
