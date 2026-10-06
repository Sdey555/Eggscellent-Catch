#include "Basket.h"
#include "Pixel.h"
#include <cmath>

void rasterizeBasket(MyPixels &pixels, const Basket &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = b.mathY;
    const int hw = b.halfWidth;

    // Handles
    pixels.setPixel(bx - hw, by + 4, QColor(93, 64, 55));
    pixels.setPixel(bx + hw, by + 4, QColor(93, 64, 55));

    // Top Rim
    pixels.setPixel(bx - hw, by + 3, QColor(109, 76, 65));
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        pixels.setPixel(x, by + 3, QColor(215, 204, 200)); // rim highlight
    }
    pixels.setPixel(bx + hw, by + 3, QColor(109, 76, 65));

    // Wicker body rows with woven pattern
    for (int y = by + 2; y >= by + 1; --y)
    {
        pixels.setPixel(bx - hw + 1, y, QColor(93, 64, 55));
        for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
        {
            if ((x + y) % 2 == 0)
            {
                pixels.setPixel(x, y, QColor(161, 136, 127));
            }
            else
            {
                pixels.setPixel(x, y, QColor(141, 110, 99));
            }
        }
        pixels.setPixel(bx + hw - 1, y, QColor(93, 64, 55));
    }

    // Base
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        pixels.setPixel(x, by, QColor(93, 64, 55));
    }
}
