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
