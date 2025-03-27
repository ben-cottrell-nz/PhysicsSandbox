#include "physworldview_skia_quickitem.h"
#include "box2d/box2d.h"
#include "box2d/types.h"
#include <QObject>
#include <QTimer>
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

struct PhysicsBody
{
    int layerId;
    b2BodyId bodyId;
    b2ShapeDef shapeDef;
    b2BodyDef bodyDef;
    b2Polygon dynamicBox;    
};

class PhysWorldViewPrivate
{
    friend class PhysWorldViewSkia;
public:
    const float kPhysTimeStep = 1.0f;
    const int kPhysSubStepCount = 2;
    PhysWorldViewPrivate()
    {
        using namespace std::chrono_literals;
        m_worldDef = b2DefaultWorldDef();
        m_worldDef.gravity = (b2Vec2){0.0f, 10.f};
        m_worldId = b2CreateWorld(&m_worldDef);
        m_groundBodyDef = b2DefaultBodyDef();
        m_groundBodyDef.position = (b2Vec2){0.0f, 500.0f};
        m_groundId = b2CreateBody(m_worldId, &m_groundBodyDef);
        m_groundBox = b2MakeBox(3000.0f, 10.f);
        m_groundShapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(m_groundId, &m_groundShapeDef, &m_groundBox);
        m_physStepTimer = new QTimer();
        m_physStepTimer->setInterval(16ms);
        m_physStepTimer->start();
        QObject::connect(m_physStepTimer, &QTimer::timeout, [&]() {
            b2World_Step(m_worldId, kPhysTimeStep, kPhysSubStepCount);
        });
    }
    ~PhysWorldViewPrivate()
    {
        if (m_physStepTimer)
            m_physStepTimer->deleteLater();
    }
    void startPhysSolver()
    {
        if (m_physStepTimer)
            m_physStepTimer->start();
    }
    void stopPhysSolver()
    {
        if (m_physStepTimer)
            m_physStepTimer->stop();
    }
    void testAddBoxes()
    {
        for (int y=0; y < 3; y++) {
            for (int x=0; x < 25; x++) {
                addPhysBox({24.f * x,-8.f+(24.f*y)+16}, 0.5*M_PI);
            }
        }

    }
    void testRemoveBoxes()
    {

    }
    void addPhysBox(b2Vec2 initialPos, float initialRot)
    {
        PhysicsBody b;
        //b2World* w = b2GetWorldFromId(m_worldId);
        //b2World_Get
        b.layerId = 1;
        b.bodyDef = b2DefaultBodyDef();
        b.bodyDef.type = b2_dynamicBody;
        b.bodyDef.position = initialPos;
        b.bodyDef.rotation = b2MakeRot(initialRot);
        b.bodyDef.angularDamping = 0;
        b.bodyDef.angularVelocity = 3.f;
        b.bodyId = b2CreateBody(m_worldId, &b.bodyDef);
        b.shapeDef = b2DefaultShapeDef();
        b.shapeDef.density = 4.f;
        b.shapeDef.friction = 0.4f;
        b.shapeDef.restitution = 0.5f;
        b.dynamicBox = b2MakeBox(14.f,14.f);
        b2CreatePolygonShape(b.bodyId, &b.shapeDef, &b.dynamicBox);
        m_physBodies.append(std::move(b));
    }
    void removePhysBody(const int index)
    {
        auto e = m_physBodies.at(index);
        b2DestroyBody(e.bodyId);
        m_physBodies.removeAt(index);
    }
    void removePhysBodies(const int layer)
    {
        auto numRemoved = m_physBodies.removeIf([&](const PhysicsBody& e) {
            return e.layerId == layer;
        });
    }
    b2ShapeId getb2BodyMainShape(const b2BodyId& id)
    {
        b2ShapeId tempShapes[2];
        b2Body_GetShapes(id, tempShapes, 2);
        return tempShapes[0];
    }
    // Must be called with an active Skia canvas
    void drawb2Body(SkCanvas* p, const b2BodyId& bodyId)
    {
        // Theres a link between masses and shapes bodies can have many shapes (which are linked)
        const int shapeCount = b2Body_GetShapeCount(bodyId);
        b2ShapeId tempShapes[shapeCount];
        b2Body_GetShapes(bodyId, tempShapes, shapeCount);
        for (int si=0; si<shapeCount; si++) {
            auto md = b2Shape_GetMassData(tempShapes[si]);
            auto bodyTransform = b2Body_GetTransform(bodyId);
            b2Polygon b2poly = b2Shape_GetPolygon(tempShapes[si]);
            SkPath shapePath;
            b2Vec2 np = b2TransformPoint(bodyTransform, b2poly.vertices[0]);
            shapePath.moveTo(np.x, np.y);
            for (int i=1; i < b2poly.count; i++) {
                np = b2TransformPoint(bodyTransform, b2poly.vertices[i]);
                shapePath.lineTo(np.x, np.y);
                p->drawPoint({np.x, np.y}, SkPaint(SkColor4f::FromColor(SK_ColorBLACK)));
            }
            shapePath.close();
            p->drawPath(shapePath, SkPaint(SkColor4f::FromColor(SK_ColorCYAN)));

        }
    }
    const QList<PhysicsBody>& getPhysBodies()
    {
        return m_physBodies;
    }
    const b2BodyId& getGroundBodyId()
    {
        return m_groundId;
    }
protected:
    bool m_isInitialized = false;
    QTimer* m_physStepTimer;
    // Physics bodies
    b2WorldDef m_worldDef;
    b2WorldId m_worldId;
    b2BodyDef m_groundBodyDef;
    b2BodyId m_groundId;
    b2Polygon m_groundBox;
    b2ShapeDef m_groundShapeDef;
    QList<PhysicsBody> m_physBodies;
};

PhysWorldViewSkia::PhysWorldViewSkia()
{
    connect(this, &PhysWorldViewSkia::physStepsTimerStartRequested, this, [&](){
        if (!m_dptr->m_physStepTimer->isActive()) m_dptr->m_physStepTimer->start();
    });
}

PhysWorldViewSkia::~PhysWorldViewSkia()
{
    m_dptr->m_physStepTimer->stop();
}

void PhysWorldViewSkia::testAddBoxes()
{
    m_dptr->testAddBoxes();
    emit physStepsTimerStartRequested();
}

void PhysWorldViewSkia::onInit(int w, int h)
{
    m_dptr.reset(new PhysWorldViewPrivate());
    //m_dptr->testAddBoxes();
}

void PhysWorldViewSkia::onResize(int w, int h)
{

}

void PhysWorldViewSkia::draw(SkCanvas *canvas, int elapsed)
{
    if (!m_dptr) return;
    if (!canvas) return;
    canvas->clear(SK_ColorWHITE);
    auto clipRect = canvas->getDeviceClipBounds();
    SkPaint f(SkColor4f::FromColor(SK_ColorGREEN));
    /*
     *
     *     p.setWindow(QRect(-(50*scaleFactor), -(50*scaleFactor),
                      100*scaleFactor, 100*scaleFactor));
    */
    double a = -500;
    double b = -500;
    double c = 1000;
    double d = 1000;
    SkMatrix matrix;
    //matrix.setScale(1.f / (clipRect.width() / (c - a)), 1.f / (clipRect.height() / (d-b)));
    //canvas->setMatrix(matrix);
    //canvas->drawIRect(clipRect, f);
    //canvas->scale(clipRect.width() / 2.f, clipRect.height() / 2.f);
    //canvas->scale(0.2,0.2);
    //canvas->translate(width() / 2, height() /2);
    // canvas->scale(
    //     0.25,
    //     0.25
    // );
    const float scaleFactor = 2.5f;
    matrix.setRectToRect({-50*scaleFactor,-50*scaleFactor,100*scaleFactor,100*scaleFactor},
                         {0,0,(float)width()/(2+ scaleFactor),(float)height()/(2+scaleFactor)},
                         SkMatrix::kCenter_ScaleToFit);
    matrix.setTranslateY(height() / 2.f);
    canvas->setMatrix(matrix);
    auto physBodies = m_dptr->getPhysBodies();
    m_dptr->drawb2Body(canvas, m_dptr->m_groundId);
    for (size_t i= 0; i < physBodies.size(); i++) {
        m_dptr->drawb2Body(canvas, physBodies[i].bodyId);
    }
}
