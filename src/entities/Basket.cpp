#include "Basket.h"
#include "Pixel.h"
#include <cmath>

void rasterizeBasket(MyPixels &pixels, const Basket &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = b.mathY;
    const int hw = b.halfWidth;

    // Handles on Left and Right (Rows by + 6 down to by + 4)
    pixels.setPixel(bx - hw, by + 6, QColor(78, 52, 46));
    pixels.setPixel(bx - hw + 1, by + 6, QColor(121, 85, 72));
    pixels.setPixel(bx - hw, by + 5, QColor(93, 64, 55));
    pixels.setPixel(bx - hw + 1, by + 4, QColor(212, 175, 55)); // Brass handle rivet

    pixels.setPixel(bx + hw, by + 6, QColor(78, 52, 46));
    pixels.setPixel(bx + hw - 1, by + 6, QColor(121, 85, 72));
    pixels.setPixel(bx + hw, by + 5, QColor(93, 64, 55));
    pixels.setPixel(bx + hw - 1, by + 4, QColor(212, 175, 55)); // Brass handle rivet

    // Top Rim: Row by + 5
    pixels.setPixel(bx - hw + 1, by + 5, QColor(141, 110, 99));
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        pixels.setPixel(x, by + 5, (x % 2 == 0) ? QColor(235, 222, 215) : QColor(215, 204, 200));
    }
    pixels.setPixel(bx + hw - 1, by + 5, QColor(141, 110, 99));

    // Raised Defensive Shield Dome over the rim when player is actively defending
    if (b.isDefending)
    {
        const QColor shieldBorder(0, 229, 255);
        const QColor shieldCore(180, 240, 255);
        const QColor shieldGlint(255, 255, 255);

        for (int x = bx - hw; x <= bx + hw; ++x)
        {
            const double norm = static_cast<double>(x - bx) / std::max(1, hw);
            const int archH = static_cast<int>(std::round((1.0 - norm * norm) * 3.5));
            for (int h = 1; h <= archH + 1; ++h)
            {
                const int sy = by + 5 + h;
                if (h == archH + 1)
                {
                    pixels.setPixel(x, sy, (x % 3 == 0) ? shieldGlint : shieldBorder);
                }
                else
                {
                    pixels.setPixel(x, sy, shieldCore);
                }
            }
        }
    }

    // Lower Rim Band: Row by + 4
    pixels.setPixel(bx - hw, by + 4, QColor(93, 64, 55));
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        pixels.setPixel(x, by + 4, QColor(109, 76, 65));
    }
    pixels.setPixel(bx + hw, by + 4, QColor(93, 64, 55));

    // Wicker Body Row 3: Row by + 3
    pixels.setPixel(bx - hw, by + 3, QColor(93, 64, 55));
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        if ((x + 3) % 2 == 0)
        {
            pixels.setPixel(x, by + 3, QColor(188, 170, 164));
        }
        else
        {
            pixels.setPixel(x, by + 3, QColor(141, 110, 99));
        }
    }
    pixels.setPixel(bx + hw, by + 3, QColor(93, 64, 55));

    // Wicker Body Row 2: Row by + 2 (tapered)
    pixels.setPixel(bx - hw + 1, by + 2, QColor(78, 52, 46));
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        if ((x + 2) % 2 == 0)
        {
            pixels.setPixel(x, by + 2, QColor(161, 136, 127));
        }
        else
        {
            pixels.setPixel(x, by + 2, QColor(121, 85, 72));
        }
    }
    pixels.setPixel(bx + hw - 1, by + 2, QColor(78, 52, 46));

    // Wicker Body Row 1: Row by + 1 (tapered)
    pixels.setPixel(bx - hw + 1, by + 1, QColor(78, 52, 46));
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        if ((x + 1) % 2 == 0)
        {
            pixels.setPixel(x, by + 1, QColor(141, 110, 99));
        }
        else
        {
            pixels.setPixel(x, by + 1, QColor(93, 64, 55));
        }
    }
    pixels.setPixel(bx + hw - 1, by + 1, QColor(78, 52, 46));

    // Base: Row by
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        pixels.setPixel(x, by, QColor(62, 39, 35));
    }
}

void rasterizeCompanionBasket(MyPixels &pixels, const CompanionBasket &comp)
{
    if (!comp.active)
    {
        return;
    }

    const int bx = static_cast<int>(std::round(comp.mathX));
    const int by = static_cast<int>(std::round(comp.mathY));
    const int hw = comp.halfWidth;

    const QColor cyanA(0, 229, 255);
    const QColor cyanB(0, 180, 216);
    const QColor goldCore(255, 215, 0);
    const QColor goldDeep(255, 160, 0);
    const QColor whiteHot(255, 255, 240);
    const QColor darkBase(20, 40, 60);

    // Fluttering clockwork side wings
    const int wingFlap = (comp.ticksLeft % 6 < 3) ? 1 : 0;
    // Left Wing
    pixels.setPixel(bx - hw - 1, by + 5 + wingFlap, whiteHot);
    pixels.setPixel(bx - hw - 2, by + 6 + wingFlap, cyanA);
    pixels.setPixel(bx - hw - 1, by + 4, cyanB);
    // Right Wing
    pixels.setPixel(bx + hw + 1, by + 5 + wingFlap, whiteHot);
    pixels.setPixel(bx + hw + 2, by + 6 + wingFlap, cyanA);
    pixels.setPixel(bx + hw + 1, by + 4, cyanB);

    // Handles
    pixels.setPixel(bx - hw, by + 6, goldDeep);
    pixels.setPixel(bx - hw + 1, by + 6, goldCore);
    pixels.setPixel(bx + hw, by + 6, goldDeep);
    pixels.setPixel(bx + hw - 1, by + 6, goldCore);

    // Top Rim: Row by + 5
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        pixels.setPixel(x, by + 5, (x % 2 == 0) ? whiteHot : cyanA);
    }

    // Lower Rim Band: Row by + 4
    for (int x = bx - hw; x <= bx + hw; ++x)
    {
        pixels.setPixel(x, by + 4, goldDeep);
    }

    // Wicker Body Row 3
    for (int x = bx - hw; x <= bx + hw; ++x)
    {
        pixels.setPixel(x, by + 3, (x % 2 == 0) ? cyanA : goldCore);
    }

    // Wicker Body Row 2 (tapered)
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        pixels.setPixel(x, by + 2, (x % 2 == 0) ? cyanB : goldDeep);
    }

    // Wicker Body Row 1 (tapered)
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        pixels.setPixel(x, by + 1, darkBase);
    }

    // Base: Row by
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        pixels.setPixel(x, by, darkBase);
    }
}
