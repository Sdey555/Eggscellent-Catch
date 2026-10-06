#include "Bird.h"
#include "Pixel.h"
#include <cmath>

void rasterizeBird(MyPixels &pixels, const Bird &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = static_cast<int>(std::round(b.mathY));
    const int dir = b.direction;

    auto drawOffset = [&](int ox, int oy, const QColor &col) {
        pixels.setPixel(bx + ox * dir, by + oy, col);
    };

    const QColor body = b.bodyColor;
    const QColor wing = b.wingColor;
    const QColor belly = b.bellyColor;
    const QColor beak(243, 156, 18);
    const QColor eye(255, 255, 255);
    const QColor pupil(20, 20, 20);

    // Beak
    drawOffset(4, 0, beak);
    drawOffset(3, 0, beak);

    // Head & Eye
    drawOffset(2, 1, body);
    drawOffset(1, 1, eye);
    drawOffset(1, 1, pupil);
    drawOffset(2, 0, body);

    // Body
    for (int ox = -2; ox <= 1; ++ox)
    {
        drawOffset(ox, 0, body);
        drawOffset(ox, -1, belly);
    }
    drawOffset(-3, 0, body);   // Tail base
    drawOffset(-4, 1, wing);   // Tail feather

    // Flapping Wing
    if (b.wingFrame == 0)
    {
        // Wing Level / Up
        drawOffset(-1, 1, wing);
        drawOffset(-2, 1, wing);
        drawOffset(-1, 2, wing);
    }
    else
    {
        // Wing Down
        drawOffset(-1, -1, wing);
        drawOffset(-2, -1, wing);
        drawOffset(-1, -2, wing);
    }

    // Warning indicator if bird is about to lay an egg
    if (b.isLaying)
    {
        const QColor alertCol = (b.layingCountdown % 4 < 2) ? QColor(255, 193, 7) : QColor(231, 76, 60);
        pixels.setPixel(bx, by - 2, alertCol);
        pixels.setPixel(bx, by - 3, alertCol);
    }
}
