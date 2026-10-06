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
    else if (egg.type == EggType::BASKET_GROW)
    {
        // 6x8 Special Emerald Basket Growth Egg with animated pulsing '+' symbol
        const QColor border(11, 83, 38);
        const QColor emerald(39, 174, 96);
        const QColor brightMint(46, 204, 113);
        const QColor shine(163, 228, 215);
        const QColor iconCol = (egg.animTick % 4 < 2) ? QColor(255, 255, 255) : brightMint;

        // Top
        pixels.setPixel(ex - 1, ey + 3, border);
        pixels.setPixel(ex, ey + 3, border);

        for (int y = ey + 2; y >= ey - 2; --y)
        {
            pixels.setPixel(ex - 2, y, border);
            pixels.setPixel(ex + 2, y, border);
        }

        // Top inner row
        pixels.setPixel(ex - 1, ey + 2, shine);
        pixels.setPixel(ex, ey + 2, brightMint);
        pixels.setPixel(ex + 1, ey + 2, emerald);

        // Row +1 with vertical arm of '+'
        pixels.setPixel(ex - 1, ey + 1, brightMint);
        pixels.setPixel(ex, ey + 1, iconCol);
        pixels.setPixel(ex + 1, ey + 1, emerald);

        // Row 0 with horizontal arms of '+'
        pixels.setPixel(ex - 1, ey, iconCol);
        pixels.setPixel(ex, ey, iconCol);
        pixels.setPixel(ex + 1, ey, iconCol);

        // Row -1 with vertical arm of '+'
        pixels.setPixel(ex - 1, ey - 1, brightMint);
        pixels.setPixel(ex, ey - 1, iconCol);
        pixels.setPixel(ex + 1, ey - 1, emerald);

        // Bottom
        pixels.setPixel(ex - 1, ey - 2, border);
        pixels.setPixel(ex, ey - 2, emerald);
        pixels.setPixel(ex + 1, ey - 2, border);
    }
    else if (egg.type == EggType::BASKET_RESTORE)
    {
        // 7x9 Legendary Prismatic Rainbow Crowned Egg (restores basket to starting size)
        const QColor goldCrown(255, 215, 0);
        const QColor goldBorder(184, 134, 11);
        const QColor diamondGlint = (egg.animTick % 4 < 2) ? QColor(255, 255, 255) : QColor(224, 247, 250);

        // Animated rainbow spectrum colors
        static const QColor spectrum[5] = {
            QColor(233, 30, 99),   // Rose Magenta
            QColor(156, 39, 176),  // Royal Violet
            QColor(0, 229, 255),   // Electric Cyan
            QColor(0, 230, 118),   // Spring Emerald
            QColor(255, 235, 59)   // Bright Gold
        };
        const int sShift = (egg.animTick / 3) % 5;

        // Golden Crowned Top: Row ey + 4
        pixels.setPixel(ex - 1, ey + 4, goldCrown);
        pixels.setPixel(ex, ey + 4, diamondGlint);
        pixels.setPixel(ex + 1, ey + 4, goldCrown);

        // Row ey + 3
        pixels.setPixel(ex - 2, ey + 3, goldBorder);
        pixels.setPixel(ex - 1, ey + 3, spectrum[(sShift + 0) % 5]);
        pixels.setPixel(ex, ey + 3, diamondGlint);
        pixels.setPixel(ex + 1, ey + 3, spectrum[(sShift + 1) % 5]);
        pixels.setPixel(ex + 2, ey + 3, goldBorder);

        // Rows ey + 2 to ey - 2: Shifting rainbow diamond body
        for (int y = ey + 2; y >= ey - 2; --y)
        {
            const int rowIdx = (ey + 2 - y);
            pixels.setPixel(ex - 3, y, goldBorder);
            pixels.setPixel(ex - 2, y, spectrum[(sShift + rowIdx) % 5]);
            pixels.setPixel(ex - 1, y, spectrum[(sShift + rowIdx + 1) % 5]);
            pixels.setPixel(ex, y, (rowIdx == 2 ? diamondGlint : spectrum[(sShift + rowIdx + 2) % 5]));
            pixels.setPixel(ex + 1, y, spectrum[(sShift + rowIdx + 3) % 5]);
            pixels.setPixel(ex + 2, y, spectrum[(sShift + rowIdx + 4) % 5]);
            pixels.setPixel(ex + 3, y, goldBorder);
        }

        // Row ey - 3
        pixels.setPixel(ex - 2, ey - 3, goldBorder);
        pixels.setPixel(ex - 1, ey - 3, spectrum[(sShift + 2) % 5]);
        pixels.setPixel(ex, ey - 3, spectrum[(sShift + 3) % 5]);
        pixels.setPixel(ex + 1, ey - 3, spectrum[(sShift + 4) % 5]);
        pixels.setPixel(ex + 2, ey - 3, goldBorder);

        // Bottom: Row ey - 4
        pixels.setPixel(ex - 1, ey - 4, goldBorder);
        pixels.setPixel(ex, ey - 4, goldCrown);
        pixels.setPixel(ex + 1, ey - 4, goldBorder);
    }
}

