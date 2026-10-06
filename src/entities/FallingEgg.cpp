#include "FallingEgg.h"
#include "Pixel.h"
#include <cmath>

void rasterizeEgg(MyPixels &pixels, const FallingEgg &egg)
{
    const int ex = static_cast<int>(std::round(egg.mathX));
    const int ey = static_cast<int>(std::round(egg.mathY));

    if (egg.type == EggType::REGULAR)
    {
        // 5x6 Regular White/Eggshell Egg
        const QColor outline(141, 110, 99);
        const QColor fill(255, 253, 240);
        const QColor shade(220, 214, 198);
        const QColor highlight(255, 255, 255);

        // Top row
        pixels.setPixel(ex - 1, ey + 2, outline);
        pixels.setPixel(ex, ey + 2, outline);
        pixels.setPixel(ex + 1, ey + 2, outline);

        // Row 1
        pixels.setPixel(ex - 2, ey + 1, outline);
        pixels.setPixel(ex - 1, ey + 1, highlight);
        pixels.setPixel(ex, ey + 1, fill);
        pixels.setPixel(ex + 1, ey + 1, fill);
        pixels.setPixel(ex + 2, ey + 1, outline);

        // Row 0
        pixels.setPixel(ex - 2, ey, outline);
        pixels.setPixel(ex - 1, ey, highlight);
        pixels.setPixel(ex, ey, fill);
        pixels.setPixel(ex + 1, ey, fill);
        pixels.setPixel(ex + 2, ey, outline);

        // Row -1
        pixels.setPixel(ex - 2, ey - 1, outline);
        pixels.setPixel(ex - 1, ey - 1, fill);
        pixels.setPixel(ex, ey - 1, fill);
        pixels.setPixel(ex + 1, ey - 1, shade);
        pixels.setPixel(ex + 2, ey - 1, outline);

        // Bottom row
        pixels.setPixel(ex - 1, ey - 2, outline);
        pixels.setPixel(ex, ey - 2, shade);
        pixels.setPixel(ex + 1, ey - 2, outline);
    }
    else if (egg.type == EggType::GOLDEN)
    {
        // 6x7 Big Shiny Golden Egg
        const QColor border(160, 90, 0);
        const QColor goldBright(255, 235, 59);
        const QColor goldCore(255, 215, 0);
        const QColor goldDeep(255, 143, 0);
        const QColor sparkle = (egg.animTick % 6 < 3) ? QColor(255, 255, 255) : goldBright;

        // Top
        pixels.setPixel(ex - 1, ey + 3, border);
        pixels.setPixel(ex, ey + 3, border);

        for (int y = ey + 2; y >= ey - 2; --y)
        {
            pixels.setPixel(ex - 2, y, border);
            pixels.setPixel(ex + 2, y, border);
        }

        // Inner golden cells with animated sparkle
        pixels.setPixel(ex - 1, ey + 2, sparkle);
        pixels.setPixel(ex, ey + 2, goldBright);
        pixels.setPixel(ex + 1, ey + 2, goldCore);

        pixels.setPixel(ex - 1, ey + 1, goldBright);
        pixels.setPixel(ex, ey + 1, goldCore);
        pixels.setPixel(ex + 1, ey + 1, goldDeep);

        pixels.setPixel(ex - 1, ey, goldCore);
        pixels.setPixel(ex, ey, goldCore);
        pixels.setPixel(ex + 1, ey, goldDeep);

        pixels.setPixel(ex - 1, ey - 1, goldCore);
        pixels.setPixel(ex, ey - 1, goldDeep);
        pixels.setPixel(ex + 1, ey - 1, goldDeep);

        // Bottom
        pixels.setPixel(ex - 1, ey - 2, border);
        pixels.setPixel(ex, ey - 2, goldDeep);
        pixels.setPixel(ex + 1, ey - 2, border);
    }
    else if (egg.type == EggType::BOMB)
    {
        // 6x7 False Egg (Bomb with flickering fuse)
        const QColor sparkColor = (egg.animTick % 4 < 2) ? QColor(255, 215, 0) : QColor(231, 76, 60);
        const QColor fuseColor(121, 85, 72);
        const QColor metalOutline(38, 50, 56);
        const QColor metalBody(55, 71, 79);
        const QColor highlight(120, 144, 156);
        const QColor dangerCross(229, 57, 53);

        // Flickering fuse on top
        pixels.setPixel(ex, ey + 3, sparkColor);
        pixels.setPixel(ex + 1, ey + 3, (egg.animTick % 3 == 0 ? sparkColor : fuseColor));
        pixels.setPixel(ex, ey + 2, fuseColor);

        // Bomb body outline
        pixels.setPixel(ex - 1, ey + 1, metalOutline);
        pixels.setPixel(ex, ey + 1, metalOutline);
        pixels.setPixel(ex + 1, ey + 1, metalOutline);

        for (int y = ey; y >= ey - 1; --y)
        {
            pixels.setPixel(ex - 2, y, metalOutline);
            pixels.setPixel(ex + 2, y, metalOutline);
        }

        // Bomb body interior with red danger cross
        pixels.setPixel(ex - 1, ey, highlight);
        pixels.setPixel(ex, ey, dangerCross);
        pixels.setPixel(ex + 1, ey, metalBody);

        pixels.setPixel(ex - 1, ey - 1, dangerCross);
        pixels.setPixel(ex, ey - 1, dangerCross);
        pixels.setPixel(ex + 1, ey - 1, dangerCross);

        // Bottom
        pixels.setPixel(ex - 1, ey - 2, metalOutline);
        pixels.setPixel(ex, ey - 2, metalOutline);
        pixels.setPixel(ex + 1, ey - 2, metalOutline);
    }
}
