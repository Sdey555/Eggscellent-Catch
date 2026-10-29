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

    // Cloud banks: bird glides into drifting cloud and disappears from sight
    if (b.inCloud)
    {
        return;
    }

    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = static_cast<int>(std::round(b.mathY));
    const int dir = b.direction;

    auto drawOffset = [&](int ox, int oy, const QColor &col) {
        pixels.setPixel(bx + ox * dir, by + oy, col);
    };

    QColor body = b.bodyColor;
    QColor wing = b.wingColor;
    QColor belly = b.bellyColor;
    QColor beakA(243, 156, 18);
    QColor beakB(230, 126, 34);
    QColor eye(255, 255, 255);
    QColor pupil(15, 23, 42);

    if (b.isSilhouette)
    {
        // Fog dive at night: dark ink silhouette cutting through mist
        body = QColor(20, 24, 38);
        wing = QColor(12, 16, 26);
        belly = QColor(28, 34, 48);
        beakA = QColor(36, 42, 58);
        beakB = QColor(26, 30, 44);

        // Faint eye glint so players can track it across the night fog
        const int glintPulse = (b.animTick % 12 < 6) ? 255 : 205;
        eye = QColor(255, 235, 130, glintPulse);   // Faint luminous amber glint
        pupil = QColor(255, 255, 215, glintPulse); // Piercing bright spark
    }

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

    // Authentic Fire Phoenix Palette (Molten Core, Blazing Amber, Searing Crimson)
    const QColor emberOutline(62, 14, 14);    // Deep smoldering dark contour
    const QColor flameDarkRed(178, 24, 24);   // Deep scorched crimson
    const QColor flameCrimson(229, 57, 53);   // Searing fire scarlet
    const QColor flameOrange(244, 81, 30);    // Roaring flame orange
    const QColor flameAmber(255, 143, 0);     // Molten amber
    const QColor flameGold(255, 193, 7);      // Radiant solar gold
    const QColor flameYellow(255, 238, 88);   // Incandescent fire yellow
    const QColor flameWhite(255, 254, 235);   // White-hot spark tip

    // Subtle living flame temperature flicker
    const int flickerTick = (b.animTick % 6);
    const QColor flameTipA = (flickerTick < 3) ? flameWhite : flameYellow;
    const QColor flameTipB = (flickerTick < 3) ? flameYellow : flameGold;
    const QColor flameFeatherCore = (flickerTick % 2 == 0) ? flameYellow : flameGold;
    const QColor flameFeatherMid = (flickerTick % 2 == 0) ? flameOrange : flameAmber;

    // 1. Royal Raptor Beak (Curved golden raptor beak: ox +8 to +11, rows +1 to +2)
    drawOffset(11, 1, flameYellow);
    drawOffset(10, 1, flameGold);
    drawOffset(10, 2, flameAmber);
    drawOffset(9, 1, flameAmber);
    drawOffset(9, 2, flameGold);
    drawOffset(8, 1, flameAmber);
    drawOffset(8, 2, flameGold);

    // 2. Crown of Rising Flames (Licking tongues of fire: ox +3 to +8, rows +5 to +7)
    drawOffset(4, 7, flameTipA);
    drawOffset(6, 7, flameTipA);
    drawOffset(8, 7, flameTipA);

    drawOffset(4, 6, flameYellow);
    drawOffset(5, 6, flameGold);
    drawOffset(6, 6, flameYellow);
    drawOffset(7, 6, flameGold);
    drawOffset(8, 6, flameYellow);

    drawOffset(3, 5, flameCrimson);
    drawOffset(4, 5, flameOrange);
    drawOffset(5, 5, flameGold);
    drawOffset(6, 5, flameGold);
    drawOffset(7, 5, flameOrange);
    drawOffset(8, 5, flameCrimson);

    // 3. Head & Piercing Molten Gold Eye (ox +3 to +8, rows +3 to +4)
    drawOffset(3, 4, flameCrimson);
    drawOffset(4, 4, flameOrange);
    drawOffset(5, 4, flameGold);
    drawOffset(6, 4, flameYellow);
    drawOffset(7, 4, flameGold);

    drawOffset(3, 3, flameOrange);
    drawOffset(4, 3, flameGold);
    drawOffset(5, 3, flameWhite);   // White sclera spark
    drawOffset(6, 3, flameAmber);   // Fierce amber-gold pupil
    drawOffset(7, 3, flameGold);

    // 4. Throat & High Neck (ox +2 to +7, row +2)
    drawOffset(2, 2, flameCrimson);
    drawOffset(3, 2, flameOrange);
    drawOffset(4, 2, flameGold);
    drawOffset(5, 2, flameYellow);
    drawOffset(6, 2, flameGold);
    drawOffset(7, 2, flameAmber);

    // 5. Fiery Body & Burning Core (ox -3 to +7, rows -1 to +1)
    // Row +1: Shoulder & upper breast
    drawOffset(-2, 1, flameCrimson);
    drawOffset(-1, 1, flameOrange);
    drawOffset(0, 1, flameOrange);
    drawOffset(1, 1, flameGold);
    drawOffset(2, 1, flameYellow);
    drawOffset(3, 1, flameYellow);
    drawOffset(4, 1, flameGold);
    drawOffset(5, 1, flameOrange);
    drawOffset(6, 1, flameCrimson);

    // Row 0: Glowing molten heart
    drawOffset(-3, 0, emberOutline);
    drawOffset(-2, 0, flameCrimson);
    drawOffset(-1, 0, flameOrange);
    drawOffset(0, 0, flameGold);
    drawOffset(1, 0, flameFeatherCore);
    drawOffset(2, 0, flameFeatherCore);
    drawOffset(3, 0, flameGold);
    drawOffset(4, 0, flameOrange);
    drawOffset(5, 0, flameCrimson);

    // Row -1: Underbelly heat shading
    drawOffset(-2, -1, flameDarkRed);
    drawOffset(-1, -1, flameCrimson);
    drawOffset(0, -1, flameOrange);
    drawOffset(1, -1, flameAmber);
    drawOffset(2, -1, flameAmber);
    drawOffset(3, -1, flameOrange);
    drawOffset(4, -1, flameCrimson);

    // 6. Magnificent Wings of Flame (Stepped fire feather tiers - 3 animated frames)
    if (b.wingFrame == 0)
    {
        // Frame 0: High Wing Flap (Majestic arch of flames reaching row +8)
        for (int ox = -1; ox <= 3; ++ox) drawOffset(ox, 3, flameGold);
        for (int ox = -2; ox <= 2; ++ox) drawOffset(ox, 4, flameFeatherMid);
        for (int ox = -3; ox <= 1; ++ox) drawOffset(ox, 5, flameCrimson);
        drawOffset(-2, 6, flameDarkRed);
        drawOffset(-1, 6, flameCrimson);
        drawOffset(0, 6, flameOrange);
        drawOffset(-2, 7, flameTipB);
        drawOffset(-1, 7, flameTipA);
        drawOffset(0, 7, flameTipB);
        drawOffset(-1, 8, flameTipA); // Searing white-hot spark apex
    }
    else if (b.wingFrame == 1)
    {
        // Frame 1: Level Glide (Broad sweeping fire wing)
        for (int ox = -5; ox <= 2; ++ox) drawOffset(ox, 2, (ox >= 0 ? flameGold : flameOrange));
        for (int ox = -4; ox <= 1; ++ox) drawOffset(ox, 3, (ox >= 0 ? flameOrange : flameCrimson));
        for (int ox = -3; ox <= 0; ++ox) drawOffset(ox, 4, flameCrimson);
        drawOffset(-2, 5, flameTipA);
        drawOffset(-1, 5, flameTipA);
    }
    else
    {
        // Frame 2: Downward Thrust (Power stroke of flames sweeping down to row -4)
        for (int ox = -2; ox <= 2; ++ox) drawOffset(ox, 1, flameGold);
        for (int ox = -3; ox <= 1; ++ox) drawOffset(ox, 0, flameOrange);
        for (int ox = -4; ox <= 0; ++ox) drawOffset(ox, -1, flameCrimson);
        for (int ox = -3; ox <= -1; ++ox) drawOffset(ox, -2, flameDarkRed);
        drawOffset(-3, -3, flameOrange);
        drawOffset(-2, -3, flameTipB);
        drawOffset(-2, -4, flameTipA); // Searing downward spark
    }

    // 7. Trailing Twin Fire Streamers (Living flame ribbons undulating back to ox = -13)
    const int waveA = static_cast<int>(std::round(std::sin((b.animTick + 0) * 0.4) * 1.5));
    const int waveB = static_cast<int>(std::round(std::sin((b.animTick + 3) * 0.4) * 1.5));

    // Upper Fire Streamer
    drawOffset(-4, 2, flameOrange);
    drawOffset(-5, 2, flameOrange);
    drawOffset(-6, 2 + waveA, flameCrimson);
    drawOffset(-7, 2 + waveA, flameCrimson);
    drawOffset(-8, 3 + waveA, flameDarkRed);
    drawOffset(-9, 3 + waveA, flameCrimson);
    drawOffset(-10, 3 + waveA, flameOrange);
    drawOffset(-11, 3 + waveA, flameGold);
    drawOffset(-12, 3 + waveA, flameTipB);
    drawOffset(-13, 4 + waveA, flameTipA); // White-hot flame spark tip

    // Lower Fire Streamer
    drawOffset(-4, 0, flameOrange);
    drawOffset(-5, 0, flameCrimson);
    drawOffset(-6, 0 + waveB, flameCrimson);
    drawOffset(-7, -1 + waveB, flameDarkRed);
    drawOffset(-8, -1 + waveB, flameCrimson);
    drawOffset(-9, -1 + waveB, flameOrange);
    drawOffset(-10, -2 + waveB, flameGold);
    drawOffset(-11, -2 + waveB, flameAmber);
    drawOffset(-12, -2 + waveB, flameTipA); // Ember spark tip

    // 8. Warning indicator when preparing to lay the legendary egg
    if (b.isLaying)
    {
        const QColor fireAlert = (b.layingCountdown % 4 < 2) ? flameWhite : flameYellow;
        pixels.setPixel(bx, by - 3, fireAlert);
        pixels.setPixel(bx, by - 4, flameOrange);
        pixels.setPixel(bx - 1, by - 3, flameGold);
        pixels.setPixel(bx + 1, by - 3, flameGold);
        pixels.setPixel(bx - 2, by - 4, flameCrimson);
        pixels.setPixel(bx + 2, by - 4, flameCrimson);
    }
}
