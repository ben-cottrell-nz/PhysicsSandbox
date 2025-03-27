#include "physworldview_original.h"
#include <QPainter>
#include <QOpenGLFunctions>
#include <random>

std::chrono::time_point<std::chrono::steady_clock> st;

PhysWorldView::PhysWorldView(QWidget *parent)
    : QOpenGLWidget(parent)
{
    using namespace std::chrono_literals;
    // Setup the world
    worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 10.f};
    worldId = b2CreateWorld(&worldDef);
    groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, 0.0f};
    groundId = b2CreateBody(worldId, &groundBodyDef);
    groundBox = b2MakeBox(500.0f, 10.f);
    groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
    timeStep = 1.0f / numSimulationFrames;
    fpsCounterLabel = new QLabel(this);
    fpsCounterLabel->setText("0 FPS");
    fpsCounterLabel->resize(300, 60);
    fpsCounterLabel->move(0, 585);
    QPushButton* btnAddBoxes = new QPushButton(this);
    btnAddBoxes->setText("Spawn Boxes");
    connect(btnAddBoxes, &QPushButton::clicked, this, [&](){
        for (int y=0; y < 6; y++) {
            for (int x=0; x < 50; x++) {
                addPhysBox({24.f * x,-280.f+(24.f*y)+16}, 0.5*M_PI);
            }
        }
        setWindowTitle(QString("PhysicsSandbox - Total Objects: %1")
                           .arg(physBodies.count() + 1));
    });
    fpsCounterTimer = new QTimer(this);
    fpsCounterTimer->setInterval(1000);
    connect(fpsCounterTimer, &QTimer::timeout, this, [&](){
        float frameTimesSum = 0;
        for (int i=0; i < 30; i++) frameTimesSum += frameTimes[i];
        fpsCounterLabel->setText(QString("%1 FPS").arg(1000.f/(frameTimesSum/30.f)));
    });
    simulateTimer = new QTimer(this);
    simulateTimer->setInterval(1000.f/numSimulationFrames);
    connect(simulateTimer, &QTimer::timeout, this, [&](){
        st = std::chrono::steady_clock::now();
        b2World_Step(worldId, timeStep, subStepCount);
        update();
    });
    simulateTimer->start();
    fpsCounterTimer->start();
}

PhysWorldView::~PhysWorldView() {
    b2DestroyWorld(worldId);
}

void PhysWorldView::initializeGL()
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    //f->glEnable(GL_MULTISAMPLE);
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void PhysWorldView::paintGL()
{
    QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
    //f->glEnable(GL_MULTISAMPLE);
    f->glClear(GL_COLOR_BUFFER_BIT);
}

b2ShapeId PhysWorldView::getb2BodyMainShape(const b2BodyId& id)
{
    b2ShapeId tempShapes[2];
    b2Body_GetShapes(id, tempShapes, 2);
    return tempShapes[0];
}

void PhysWorldView::drawb2Body(QPainter& p, const b2BodyId& bodyId)
{
    // Theres a link between masses and shapes
    // bodies can have many shapes (that are linked)
    const int shapeCount = b2Body_GetShapeCount(bodyId);
    b2ShapeId tempShapes[shapeCount];
    b2Body_GetShapes(bodyId, tempShapes, shapeCount);
    for (int si=0; si<shapeCount; si++) {
        auto md = b2Shape_GetMassData(tempShapes[si]);
        auto bodyTransform = b2Body_GetTransform(bodyId);
        QList<QPointF> pts;
        b2Polygon b2poly = b2Shape_GetPolygon(tempShapes[si]);
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
        p.setBrush(QBrush(QColor(0x542c2c)));
        p.save();
        p.setPen(QPen(QBrush(Qt::GlobalColor::black), 5.f));
        p.drawPoints(drawPolygon);
        p.restore();
    }
    frameTimes[frameTimesCounter] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - st).count();
    frameTimesCounter += 1;
    frameTimesCounter = frameTimesCounter % 30;
    //frameTimeElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - st).count();
}

void PhysWorldView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter p(this);
    p.fillRect(rect(), Qt::GlobalColor::white);
    p.setRenderHint(QPainter::RenderHint::Antialiasing);
    const int scaleFactor = 10;
    // World space: far left, far top, full width, full height
    p.setWindow(QRect(-(50*scaleFactor), -(50*scaleFactor),
                      100*scaleFactor, 100*scaleFactor));
    for (auto &b:physBodies) {
        drawb2Body(p, b.bodyId);
    }
    drawb2Body(p, groundId);
}



// Adds a physics object to the world
void PhysWorldView::addPhysBox(b2Vec2 initialPos, float initialRot)
{
    PhysicsBody b;
    b.bodyDef = b2DefaultBodyDef();
    b.bodyDef.type = b2_dynamicBody;
    b.bodyDef.position = initialPos;
    b.bodyDef.rotation = b2MakeRot(initialRot);
    b.bodyDef.angularDamping = 0;
    b.bodyDef.angularVelocity = 3.f;
    b.bodyId = b2CreateBody(worldId, &b.bodyDef);
    b.shapeDef = b2DefaultShapeDef();
    b.shapeDef.density = 4.f;
    b.shapeDef.friction = 0.4f;
    b.shapeDef.restitution = 0.5f;
    b.dynamicBox = b2MakeBox(14.f,14.f);
    b2CreatePolygonShape(b.bodyId, &b.shapeDef, &b.dynamicBox);
    physBodies.append(std::move(b));
}

void PhysWorldView::removePhysBox(const int index)
{
    auto e = physBodies.at(index);
    b2DestroyBody(e.bodyId);
    physBodies.removeAt(index);
}
