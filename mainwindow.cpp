#include "mainwindow.h"
#include "box2d/box2d.h"
#include <QChronoTimer>
#include <QPainter>
#include <QPainterPath>

b2WorldDef worldDef;
b2WorldId worldId;
b2BodyDef groundBodyDef;
b2BodyId groundId;
b2Polygon groundBox;
b2ShapeDef groundShapeDef;
b2BodyDef bodyDef;
b2BodyId bodyId;
b2Polygon dynamicBox;
b2ShapeDef shapeDef;

float timeStep;
// This is used by the constraint solver; tradeoff between
// speed and simulation accuracy
int subStepCount = 4;
const int numSimulationFrames = 60;

QTimer* simulateTimer;
const float drawScale = 0.25f;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    using namespace std::chrono_literals;

    // Setup the world
    worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 4.f};
    worldId = b2CreateWorld(&worldDef);
    groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, 0.0f};
    groundId = b2CreateBody(worldId, &groundBodyDef);
    groundBox = b2MakeBox(500.0f, 10.f);
    groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
    timeStep = 1.0f / numSimulationFrames;
    // Setup the objects
    bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){0.0f,-80.0f};
    bodyDef.rotation = b2MakeRot(0.25*M_PI);
    bodyDef.angularDamping = 0;
    bodyDef.angularVelocity = 3.f;
    bodyId = b2CreateBody(worldId, &bodyDef);
    dynamicBox = b2MakeBox(14.0f, 14.0f);
    shapeDef = b2DefaultShapeDef();
    shapeDef.density = 0.00005f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.5f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
    simulateTimer = new QTimer(this);
    simulateTimer->setInterval(1000/(float)numSimulationFrames);
    connect(simulateTimer, &QTimer::timeout, this, [&](){
        for (int f=0; f < numSimulationFrames; f++) {
            b2World_Step(worldId, timeStep, subStepCount);
            repaint();
            //update();
        }
    });
    simulateTimer->start();
}

MainWindow::~MainWindow() {
    b2DestroyWorld(worldId);
}

b2ShapeId getb2BodyMainShape(const b2BodyId& id)
{
    b2ShapeId tempShapes[2];
    b2Body_GetShapes(bodyId, tempShapes, 2);
    return tempShapes[0];
}

void drawb2Body(QPainter& p, const b2BodyId& bodyId)
{
    b2Polygon b2poly = b2Shape_GetPolygon(getb2BodyMainShape(bodyId));
    // Theres a link between masses and shapes
    auto md = b2Shape_GetMassData(getb2BodyMainShape(bodyId));
    auto bodyTransform = b2Body_GetTransform(bodyId);
    QTransform m;
    QPointF center{md.center.x, md.center.y};
    QList<QPointF> pts;
    pts.reserve(b2poly.count);
    for (int i=0; i < b2poly.count; i++) {
        b2Vec2 np = b2TransformPoint(bodyTransform, b2poly.vertices[i]);
        //pts.push_back({b2poly.vertices[i].x,b2poly.vertices[i].y});
        QPointF npf{np.x,np.y};
        pts.push_back(npf);
        /*
        p.drawText(npf,
                   QString("%1:%2,%3")
                       .number(i).number(npf.x(),'g',3).number(npf.y(),'g',3));
        */
    }
    //pts.push_back(pts[0]);
    auto drawPolygon = QPolygonF::fromList(pts);
    p.drawConvexPolygon(drawPolygon);
    p.save();
    p.setPen(QPen(QBrush(Qt::GlobalColor::black), 5.f));
    p.drawPoints(drawPolygon);
    p.restore();
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.setRenderHint(QPainter::RenderHint::Antialiasing);
    const int scaleFactor = 5;
    // World space: far left, far top, full width, full height
    p.setWindow(QRect(-(50*scaleFactor), -(50*scaleFactor),
                      100*scaleFactor, 100*scaleFactor));
    b2Vec2 position = b2Body_GetPosition(bodyId);
    b2Rot rotation = b2Body_GetRotation(bodyId);
    //drawb2Shape(p, groundShapeDef)
    drawb2Body(p, bodyId);
    drawb2Body(p, groundId);
}
