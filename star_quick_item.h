#ifndef STAR_QUICK_ITEM_H
#define STAR_QUICK_ITEM_H
#include "skia_quick_item.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"


class StarQuickItem : public SkiaQuickItem
{
    Q_OBJECT
private:
    int m_timeCounter;
public:
    virtual void onInit(int w, int h) override
    {

    }
    virtual void onResize(int w, int h) override {}
    virtual void draw(SkCanvas* canvas, int elapsed) override
    {
        m_timeCounter += elapsed;
        m_timeCounter = m_timeCounter % 1000;
        auto size = this->size().toSize();
        canvas->clear(SkColor4f::FromColor(SK_ColorWHITE));
        // Straight from the demo code (https://fiddle.skia.org/c/@shapes)

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);
        paint.setStrokeWidth(m_timeCounter / 1000.f * 8);
        paint.setColor(0xff4285F4);

        SkRect rect = SkRect::MakeXYWH(10, 10, 100 * sin(m_timeCounter / 1000.f * M_PI), 160);
        canvas->drawRect(rect, paint);

        SkRRect oval;
        oval.setOval(rect);
        oval.offset(40, 80);
        paint.setColor(0xffDB4437);
        canvas->drawRRect(oval, paint);

        paint.setColor(0xff0F9D58);
        canvas->drawCircle(180, 50, 25, paint);

        rect.offset(80, 50);
        paint.setColor(0xffF4B400);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRoundRect(rect, 10, 10, paint);

    }
};


#endif // STAR_QUICK_ITEM_H
