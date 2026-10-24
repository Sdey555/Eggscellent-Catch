#include "Bird.h"
#include "Pixel.h"
#include <cmath>

void rasterizeLegendaryBird(MyPixels &pixels, const Bird &b);

void rasterizeBird(MyPixels &pixels, const Bird &b)
{
    if (b.isLegendary)
    {
        rasterizeLegendaryBird(pixels, b);
        return;
    }

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

void rasterizeLegendaryBird(MyPixels &pixels, const Bird &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = static_cast<int>(std::round(b.mathY));
    const int dir = b.direction;

    auto drawOffset = [&](int ox, int oy, const QColor &col) {
        pixels.setPixel(bx + ox * dir, by + oy, col);
    };

    // Shifting prismatic celestial colors matching the legendary restoration egg
    static const QColor spectrum[5] = {
        QColor(233, 30, 99),   // Rose Magenta
        QColor(156, 39, 176),  // Royal Violet
        QColor(0, 229, 255),   // Electric Cyan
        QColor(0, 230, 118),   // Spring Emerald
        QColor(255, 215, 0)    // Bright Gold
    };
    const int sShift = (b.animTick / 3) % 5;

    const QColor goldCrown(255, 215, 0);
    const QColor goldDeep(230, 126, 34);
    const QColor goldBright(255, 235, 59);
    const QColor diamondGlint = (b.animTick % 4 < 2) ? QColor(255, 255, 255) : QColor(224, 247, 250);
    const QColor eyeCyan(0, 229, 255);
    const QColor rubyGem(231, 76, 60);

    // 1. Royal Raptor Beak (Extended and sharp: ox +8 to +11, rows +1 to +2)
    drawOffset(11, 1, goldBright);
    drawOffset(10, 1, goldBright);
    drawOffset(10, 2, goldDeep);
    drawOffset(9, 1, goldDeep);
    drawOffset(9, 2, goldBright);
    drawOffset(8, 1, goldDeep);
    drawOffset(8, 2, goldBright);

    // 2. Crown Crest with Shimmering Diamond Stars (ox +3 to +8, rows +5 to +7)
    drawOffset(4, 7, diamondGlint);
    drawOffset(6, 7, diamondGlint);
    drawOffset(8, 7, diamondGlint);

    drawOffset(4, 6, spectrum[(sShift + 0) % 5]);
    drawOffset(5, 6, goldCrown);
    drawOffset(6, 6, spectrum[(sShift + 1) % 5]);
    drawOffset(7, 6, goldCrown);
    drawOffset(8, 6, spectrum[(sShift + 2) % 5]);

    drawOffset(3, 5, rubyGem);
    drawOffset(4, 5, goldCrown);
    drawOffset(5, 5, goldCrown);
    drawOffset(6, 5, goldCrown);
    drawOffset(7, 5, goldCrown);
    drawOffset(8, 5, rubyGem);

    // 3. Royal Head & Glowing Luminous Cyan Eye (ox +3 to +8, rows +3 to +4)
    drawOffset(3, 4, spectrum[(sShift + 3) % 5]);
    drawOffset(4, 4, goldCrown);
    drawOffset(5, 4, goldBright);
    drawOffset(6, 4, goldBright);
    drawOffset(7, 4, goldCrown);

    drawOffset(3, 3, spectrum[(sShift + 2) % 5]);
    drawOffset(4, 3, goldCrown);
    drawOffset(5, 3, QColor(255, 255, 255)); // White eye ring
    drawOffset(6, 3, eyeCyan);                // Luminous cyan pupil
    drawOffset(7, 3, goldCrown);

    // 4. Neck & High Throat (ox +2 to +7, row +2)
    drawOffset(2, 2, spectrum[(sShift + 4) % 5]);
    drawOffset(3, 2, spectrum[(sShift + 3) % 5]);
    drawOffset(4, 2, goldCrown);
    drawOffset(5, 2, goldCrown);
    drawOffset(6, 2, goldBright);
    drawOffset(7, 2, goldDeep);

    // 5. Plump Radiant Body & Underbelly (ox -3 to +7, rows -1 to +1)
    for (int ox = -2; ox <= 6; ++ox)
    {
        drawOffset(ox, 1, spectrum[(sShift + (ox + 4)) % 5]);
    }
    for (int ox = -3; ox <= 5; ++ox)
    {
        drawOffset(ox, 0, goldCrown);
    }
    for (int ox = -2; ox <= 4; ++ox)
    {
        drawOffset(ox, -1, goldDeep);
    }

    // 6. Magnificent Broad Prismatic Wings (3 animated frames)
    if (b.wingFrame == 0)
    {
        // Frame 0: High Wing Flap (Majestic arch up to row +8)
        for (int ox = -1; ox <= 3; ++ox) drawOffset(ox, 3, spectrum[(sShift + 1) % 5]);
        for (int ox = -2; ox <= 2; ++ox) drawOffset(ox, 4, spectrum[(sShift + 2) % 5]);
        for (int ox = -3; ox <= 1; ++ox) drawOffset(ox, 5, spectrum[(sShift + 3) % 5]);
        drawOffset(-2, 6, spectrum[(sShift + 4) % 5]);
        drawOffset(-1, 6, spectrum[(sShift + 0) % 5]);
        drawOffset(0, 6, diamondGlint);
        drawOffset(-2, 7, diamondGlint);
        drawOffset(-1, 7, spectrum[(sShift + 1) % 5]);
        drawOffset(0, 7, diamondGlint);
        drawOffset(-1, 8, diamondGlint);
    }
    else if (b.wingFrame == 1)
    {
        // Frame 1: Level Glide (Sweeping horizontal wing span)
        for (int ox = -5; ox <= 2; ++ox) drawOffset(ox, 2, spectrum[(sShift + 1) % 5]);
        for (int ox = -4; ox <= 1; ++ox) drawOffset(ox, 3, spectrum[(sShift + 2) % 5]);
        for (int ox = -3; ox <= 0; ++ox) drawOffset(ox, 4, spectrum[(sShift + 3) % 5]);
        drawOffset(-2, 5, diamondGlint);
        drawOffset(-1, 5, diamondGlint);
    }
    else
    {
        // Frame 2: Downward Thrust (Power stroke sweeping down to row -4)
        for (int ox = -2; ox <= 2; ++ox) drawOffset(ox, 1, spectrum[(sShift + 1) % 5]);
        for (int ox = -3; ox <= 1; ++ox) drawOffset(ox, 0, spectrum[(sShift + 2) % 5]);
        for (int ox = -4; ox <= 0; ++ox) drawOffset(ox, -1, spectrum[(sShift + 3) % 5]);
        for (int ox = -3; ox <= -1; ++ox) drawOffset(ox, -2, spectrum[(sShift + 4) % 5]);
        drawOffset(-3, -3, spectrum[(sShift + 0) % 5]);
        drawOffset(-2, -3, diamondGlint);
        drawOffset(-2, -4, diamondGlint);
    }

    // 7. Trailing Phoenix Twin Streamer Tails (Undulating ribbons back to ox = -13)
    const int waveA = static_cast<int>(std::round(std::sin((b.animTick + 0) * 0.4) * 1.5));
    const int waveB = static_cast<int>(std::round(std::sin((b.animTick + 3) * 0.4) * 1.5));

    // Upper Streamer Ribbon
    drawOffset(-4, 2, spectrum[(sShift + 0) % 5]);
    drawOffset(-5, 2, spectrum[(sShift + 1) % 5]);
    drawOffset(-6, 2 + waveA, spectrum[(sShift + 2) % 5]);
    drawOffset(-7, 2 + waveA, spectrum[(sShift + 3) % 5]);
    drawOffset(-8, 3 + waveA, spectrum[(sShift + 4) % 5]);
    drawOffset(-9, 3 + waveA, spectrum[(sShift + 0) % 5]);
    drawOffset(-10, 3 + waveA, goldCrown);
    drawOffset(-11, 3 + waveA, goldCrown);
    drawOffset(-12, 3 + waveA, diamondGlint);
    drawOffset(-13, 4 + waveA, diamondGlint);

    // Lower Streamer Ribbon
    drawOffset(-4, 0, spectrum[(sShift + 2) % 5]);
    drawOffset(-5, 0, spectrum[(sShift + 3) % 5]);
    drawOffset(-6, 0 + waveB, spectrum[(sShift + 4) % 5]);
    drawOffset(-7, -1 + waveB, spectrum[(sShift + 0) % 5]);
    drawOffset(-8, -1 + waveB, spectrum[(sShift + 1) % 5]);
    drawOffset(-9, -1 + waveB, goldCrown);
    drawOffset(-10, -2 + waveB, goldCrown);
    drawOffset(-11, -2 + waveB, rubyGem);
    drawOffset(-12, -2 + waveB, diamondGlint);

    // 8. Warning indicator when preparing to lay the legendary egg
    if (b.isLaying)
    {
        pixels.setPixel(bx, by - 3, diamondGlint);
        pixels.setPixel(bx, by - 4, spectrum[sShift]);
        pixels.setPixel(bx - 1, by - 3, spectrum[(sShift + 1) % 5]);
        pixels.setPixel(bx + 1, by - 3, spectrum[(sShift + 2) % 5]);
        pixels.setPixel(bx - 2, by - 4, goldCrown);
        pixels.setPixel(bx + 2, by - 4, goldCrown);
    }
}
