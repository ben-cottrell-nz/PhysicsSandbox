#ifndef PHYSWORLDVIEW_SKIA_QUICKITEM_H
#define PHYSWORLDVIEW_SKIA_QUICKITEM_H

#include "skia_quick_item.h"

class PhysWorldViewPrivate;

class PhysWorldViewSkia : public SkiaQuickItem
{
    Q_OBJECT
public:
    PhysWorldViewSkia();
    ~PhysWorldViewSkia();
    Q_INVOKABLE void testAddBoxes();
    virtual void onInit(int w, int h) override;
    virtual void onResize(int w, int h) override;
    virtual void draw(SkCanvas* canvas, int elapsed) override;
signals:
    void physStepsTimerStartRequested();
private:
    std::shared_ptr<PhysWorldViewPrivate> m_dptr;
};

#endif // PHYSWORLDVIEW_SKIA_QUICKITEM_H
