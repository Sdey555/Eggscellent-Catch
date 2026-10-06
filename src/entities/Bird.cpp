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
    const QColor beakA(243, 156, 18);
    const QColor beakB(230, 126, 34);
    const QColor eye(255, 255, 255);
    const QColor pupil(15, 23, 42);

    // 1. Sharp beak (Offsets +6 to +8)
    drawOffset(8, 0, beakA);
    drawOffset(7, 0, beakA);
    drawOffset(7, 1, beakB);
    drawOffset(6, 0, beakB);
    drawOffset(6, 1, beakA);

    // 2. Feather crest on head top
    drawOffset(2, 4, wing);
    drawOffset(3, 4, body.lighter(115));
    drawOffset(4, 4, wing);

    // 3. Head & Eye (Offsets +3 to +6, rows +2 to +3)
    drawOffset(3, 3, body);
    drawOffset(4, 3, body);
    drawOffset(5, 3, body);

    drawOffset(3, 2, body);
    drawOffset(4, 2, eye);
    drawOffset(5, 2, pupil);
    drawOffset(6, 2, body);

    // 4. Neck & Upper Back
    drawOffset(2, 1, body);
    drawOffset(1, 1, body);
    drawOffset(0, 1, body);
    drawOffset(-1, 1, body);
    drawOffset(2, 2, body);
    drawOffset(1, 2, body);

    // 5. Plump Breast & Belly (Offsets -2 to +5, rows -1 to 0)
    for (int ox = -2; ox <= 5; ++ox)
    {
        drawOffset(ox, 0, belly);
    }
    drawOffset(-1, -1, belly.darker(115));
    drawOffset(0, -1, belly);
    drawOffset(1, -1, belly);
    drawOffset(2, -1, belly);
    drawOffset(3, -1, belly.darker(115));

    // 6. Layered Tail Feathers (Offsets -7 to -3)
    drawOffset(-4, 2, wing);
    drawOffset(-5, 2, wing);
    drawOffset(-6, 3, wing);

    drawOffset(-3, 1, body);
    drawOffset(-4, 1, body);
    drawOffset(-5, 1, wing);
    drawOffset(-6, 1, wing);
    drawOffset(-7, 2, wing.darker(120));

    drawOffset(-3, 0, body);
    drawOffset(-4, 0, wing);
    drawOffset(-5, 0, wing.darker(120));

    // 7. Flapping Wing (3 dynamic animation frames)
    if (b.wingFrame == 0)
    {
        // Wing High (Flap Up)
        drawOffset(1, 2, wing);
        drawOffset(0, 2, wing);
        drawOffset(-1, 2, wing);

        drawOffset(1, 3, wing);
        drawOffset(0, 3, wing);
        drawOffset(-1, 3, wing);
        drawOffset(-2, 3, wing);

        drawOffset(0, 4, wing);
        drawOffset(-1, 4, wing);
        drawOffset(-2, 4, wing);

        drawOffset(0, 5, wing.lighter(135));
        drawOffset(-1, 5, wing.lighter(135));
    }
    else if (b.wingFrame == 1)
    {
        // Wing Mid (Level Glide)
        for (int ox = -3; ox <= 1; ++ox)
        {
            drawOffset(ox, 1, wing);
        }
        drawOffset(-2, 2, wing);
        drawOffset(-1, 2, wing);
        drawOffset(0, 2, wing);

        drawOffset(-1, 3, wing.lighter(125));
        drawOffset(0, 3, wing.lighter(125));
    }
    else
    {
        // Wing Down (Push Down)
        drawOffset(0, 1, wing);
        drawOffset(-1, 1, wing);

        drawOffset(0, 0, wing);
        drawOffset(-1, 0, wing);
        drawOffset(-2, 0, wing);

        drawOffset(-1, -1, wing);
        drawOffset(-2, -1, wing);
        drawOffset(-3, -1, wing);

        drawOffset(-1, -2, wing.lighter(120));
        drawOffset(-2, -2, wing.lighter(120));
    }

    // 8. Warning indicator when bird is about to lay an egg
    if (b.isLaying)
    {
        const QColor alertCol = (b.layingCountdown % 4 < 2) ? QColor(255, 215, 0) : QColor(231, 76, 60);
        pixels.setPixel(bx, by - 2, alertCol);
        pixels.setPixel(bx, by - 3, alertCol);
        pixels.setPixel(bx - 1, by - 2, alertCol);
        pixels.setPixel(bx + 1, by - 2, alertCol);
    }
}
