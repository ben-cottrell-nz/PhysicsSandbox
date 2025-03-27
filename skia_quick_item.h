#ifndef SKIA_QUICK_ITEM_H
#define SKIA_QUICK_ITEM_H
// Source: https://github.com/QtSkia/QtSkia/blob/master/QtSkia/QtSkiaQuick/QuickItem/QSkiaQuickItem.h
#include <QQuickItem>
class SkCanvas;
class SkiaQuickItemPrivate;
class SkiaQuickItem : public QQuickItem {
    Q_OBJECT
public:
    SkiaQuickItem(QQuickItem* parent = nullptr);
    virtual ~SkiaQuickItem() override;
    //after skia init. Note: this function work in Skia Renderer Thread.
    virtual void onInit(int w, int h) = 0;
    //after skia resize. Note: this function work in Skia Renderer Thread.
    virtual void onResize(int w, int h) = 0;
    //draw. Note: this function work in Skia Renderer Thread.
    virtual void draw(SkCanvas* canvas, int elapsed) = 0;

protected:
    QSGNode* updatePaintNode(QSGNode* node, UpdatePaintNodeData* data) override;

private:
    SkiaQuickItemPrivate *m_dptr;
};
#endif // SKIA_QUICK_ITEM_H
