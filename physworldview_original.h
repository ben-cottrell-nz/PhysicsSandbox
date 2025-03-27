#ifndef PHYSWORLDVIEW_ORIGINAL_H
#define PHYSWORLDVIEW_ORIGINAL_H

#include <QObject>
#include <QOpenGLWidget>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include "box2d/box2d.h"

struct PhysicsBody
{
    b2BodyId bodyId;
    b2ShapeDef shapeDef;
    b2BodyDef bodyDef;
    b2Polygon dynamicBox;
};

class PhysWorldView : public QOpenGLWidget
{
    Q_OBJECT
public:
    PhysWorldView(QWidget*);
    ~PhysWorldView();
protected:
    void paintEvent(QPaintEvent*);
    void initializeGL() override;
    void paintGL() override;
private:
    void testAddBoxes();
    void testRemoveBoxes();
    void addPhysBox(b2Vec2 initialPos, float initialRot);
    void removePhysBox(const int);
    b2ShapeId getb2BodyMainShape(const b2BodyId& id);
    void drawb2Body(QPainter& p, const b2BodyId& bodyId);
    b2WorldDef worldDef;
    b2WorldId worldId;

    b2BodyDef groundBodyDef;
    b2BodyId groundId;
    b2Polygon groundBox;
    b2ShapeDef groundShapeDef;

    QList<PhysicsBody> physBodies;

    b2BodyId bodyId;
    b2ShapeDef shapeDef;
    b2BodyDef bodyDef;
    b2Polygon dynamicBox;

    float timeStep;
    // This is used by the constraint solver; tradeoff between
    // speed and simulation accuracy
    int subStepCount = 2;
    const int numSimulationFrames = 60;
    const float drawScale = 0.25f;
    int processedFrames = 0;

    int frameTimes[30] = {0};
    int frameTimesCounter = 0;
    QTimer* simulateTimer;
    QTimer* fpsCounterTimer;
    QLabel* fpsCounterLabel;
};

#endif // PHYSWORLDVIEW_ORIGINAL_H
