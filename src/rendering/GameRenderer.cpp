#include "GameRenderer.h"
#include "GameEngine.h"
#include "Grid.h"
#include "Pixel.h"
#include "Basket.h"
#include "Bird.h"
#include "FallingEgg.h"
#include "GameTypes.h"
#include "Particle.h"
#include <QRadialGradient>
#include <QFont>
#include <QRect>
#include <algorithm>
#include <cmath>

GameRenderer::GameRenderer()
{
}

void GameRenderer::paintCell(QPainter &painter,
                             const MyGrid &grid,
                             int scale,
                             int width,
                             int height,
                             int mathX,
                             int mathY,
                             const QColor &color)
{
    const int sx = grid.mathToScreenX(mathX);
    const int sy = grid.mathToScreenY(mathY);
    if (sx + scale < 0 || sx >= width || sy + scale < 0 || sy >= height)
    {
        return;
    }
    // 1. Solid raster cell without any grid-line gaps
    painter.fillRect(sx, sy, scale, scale, color);

    // 2. Inner phosphor glow highlight for vibrant arcade pixel appearance
    if (scale >= 4)
    {
        const int inset = (scale >= 8 ? 2 : 1);
        const int innerSize = scale - 2 * inset;
        if (innerSize >= 1)
        {
            QColor centerColor = color.lighter(130);
            painter.fillRect(sx + inset, sy + inset, innerSize, innerSize, centerColor);
        }
    }
}

void GameRenderer::rasterizeGround(MyPixels &pixels, int minX, int maxX, int gY, int minY)
{
    for (int x = minX; x <= maxX; ++x)
    {
        // Highlighted lush foreground grass (vibrant & sharp)
        pixels.setPixel(x, gY, ((x % 3 == 0) ? QColor(88, 214, 141) : QColor(46, 204, 113)));
        pixels.setPixel(x, gY - 1, QColor(39, 174, 96));

        // Rich fertile earth layers
        for (int y = gY - 2; y >= minY; --y)
        {
            if ((x * 7 + y * 13) % 11 == 0)
            {
                pixels.setPixel(x, y, QColor(141, 110, 99)); // Pebble speckle
            }
            else if (y > gY - 5)
            {
                pixels.setPixel(x, y, QColor(109, 76, 65));
            }
            else
            {
                pixels.setPixel(x, y, QColor(78, 52, 46));
            }
        }
    }
}

void GameRenderer::rasterizePixelHearts(MyPixels &pixels, int startX, int startY, int currentHearts, int maxHearts)
{
    // Draw 3 5x5 pixel hearts at upper left
    for (int h = 0; h < maxHearts; ++h)
    {
        const int hx = startX + h * 7;
        const int hy = startY;
        const bool active = (h < currentHearts);
        const QColor heartCol = active ? QColor(239, 71, 111) : QColor(70, 78, 96);

        // 5x5 Heart Pattern
        pixels.setPixel(hx - 1, hy, heartCol);
        pixels.setPixel(hx + 1, hy, heartCol);

        pixels.setPixel(hx - 2, hy - 1, heartCol);
        pixels.setPixel(hx - 1, hy - 1, heartCol);
        pixels.setPixel(hx, hy - 1, heartCol);
        pixels.setPixel(hx + 1, hy - 1, heartCol);
        pixels.setPixel(hx + 2, hy - 1, heartCol);

        pixels.setPixel(hx - 1, hy - 2, heartCol);
        pixels.setPixel(hx, hy - 2, heartCol);
        pixels.setPixel(hx + 1, hy - 2, heartCol);

        pixels.setPixel(hx, hy - 3, heartCol);
    }
}

void GameRenderer::rasterizeLevelBar(MyPixels &pixels, int minX, int maxX, int topY, int level, int score, int hudTick)
{
    const bool maxed = level >= GameEngine::MAX_LEVEL;
    const int levelStart = (level - 1) * GameEngine::POINTS_PER_LEVEL;
    const int earned = maxed ? GameEngine::POINTS_PER_LEVEL : std::clamp(score - levelStart, 0, GameEngine::POINTS_PER_LEVEL);

    int barX0 = minX;
    int barX1 = maxX;
    if (barX1 - barX0 < 6)
    {
        return;
    }

    const QColor border(94, 72, 140);
    const QColor emptyCol(38, 44, 62);
    const QColor notchCol(28, 32, 48);
    const QColor fillA = maxed ? QColor(255, 143, 0) : QColor(123, 44, 191);
    const QColor fillB = maxed ? QColor(255, 215, 0) : QColor(199, 125, 255);

    // Border frame
    for (int x = barX0 + 1; x <= barX1 - 1; ++x)
    {
        pixels.setPixel(x, topY, border);
        pixels.setPixel(x, topY - 4, border);
    }
    for (int y = topY - 1; y >= topY - 3; --y)
    {
        pixels.setPixel(barX0, y, border);
        pixels.setPixel(barX1, y, border);
    }

    const int innerX0 = barX0 + 1;
    const int innerX1 = barX1 - 1;
    const int innerW = innerX1 - innerX0 + 1;
    const int filledW = (innerW * earned) / GameEngine::POINTS_PER_LEVEL;
    const int shimmerX = innerX0 + (filledW > 0 ? (hudTick / 2) % (filledW + 8) : -100);

    for (int i = 0; i < innerW; ++i)
    {
        const int x = innerX0 + i;
        if (i < filledW)
        {
            const double t = innerW > 1 ? static_cast<double>(i) / (innerW - 1) : 1.0;
            QColor c(static_cast<int>(fillA.red() + (fillB.red() - fillA.red()) * t),
                     static_cast<int>(fillA.green() + (fillB.green() - fillA.green()) * t),
                     static_cast<int>(fillA.blue() + (fillB.blue() - fillA.blue()) * t));
            if (std::abs(x - shimmerX) <= 1)
            {
                c = c.lighter(140);
            }
            pixels.setPixel(x, topY - 1, c.lighter(125)); // top highlight row
            pixels.setPixel(x, topY - 2, c);
            pixels.setPixel(x, topY - 3, c.darker(125));  // bottom shade row
        }
        else
        {
            const bool notch = ((i * 10) % innerW) < 10 && i > 0;
            const QColor c = notch ? notchCol : emptyCol;
            for (int y = topY - 1; y >= topY - 3; --y)
            {
                pixels.setPixel(x, y, c);
            }
        }
    }
}

// -------------------------------------------------------------
// Pure Pixelated Background Layers (Zero Smooth Drawing)
// Brighter & Hazier Atmospheric Palette with Perspective Knoll
// -------------------------------------------------------------
void GameRenderer::renderPixelSky(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                  int minMathX, int maxMathX, int minMathY, int maxMathY, int groundMathY)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(minMathY);

    // Stepped pixel color bands: brighter, soft atmospheric pastel daylight haze
    static const QColor skyBands[] = {
        QColor(92, 138, 186),  // Atmospheric cornflower (zenith)
        QColor(112, 154, 198), // Soft airy blue
        QColor(133, 172, 210), // Pale cerulean
        QColor(154, 188, 220), // Light sky blue
        QColor(176, 204, 229), // Hazy pastel blue
        QColor(196, 218, 237), // Soft atmospheric haze
        QColor(214, 229, 243), // Pale misty sky
        QColor(228, 237, 246), // Warm pearl haze
        QColor(241, 238, 228), // Soft golden horizon haze
        QColor(246, 233, 218)  // Gentle peach horizon
    };
    const int bandCount = 10;
    const int bandH = std::max(4, (maxMathY - groundMathY) / bandCount);

    for (int b = 0; b < bandCount; ++b)
    {
        const int bandTop = maxMathY - b * bandH;
        const int bandBottom = (b == bandCount - 1) ? (groundMathY + 1) : (bandTop - bandH + 1);
        const QColor bandCol = skyBands[b];
        const QColor nextCol = (b + 1 < bandCount) ? skyBands[b + 1] : bandCol;

        // Draw solid pixel block for this band
        for (int my = bandTop; my >= bandBottom; --my)
        {
            // Transition row: retro checkerboard dither pattern
            if (my == bandBottom && b + 1 < bandCount)
            {
                for (int mx = minMathX; mx <= maxMathX; ++mx)
                {
                    const QColor c = ((mx + my) % 2 == 0) ? nextCol : bandCol;
                    const int sx = grid.mathToScreenX(mx);
                    const int sy = grid.mathToScreenY(my);
                    painter.fillRect(sx, sy, scale, scale, c);
                }
            }
            else
            {
                const int sx = grid.mathToScreenX(minMathX);
                const int sy = grid.mathToScreenY(my);
                const int sw = (maxMathX - minMathX + 1) * scale;
                painter.fillRect(sx, sy, sw, scale, bandCol);
            }
        }
    }
}

void GameRenderer::renderPixelClouds(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                    int minMathX, int maxMathX, int maxMathY, int hudTick)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    const int mathSpan = maxMathX - minMathX + 90;

    // 4 Grand Drifting Pixel Clouds (Noticeably larger and puffier)
    struct CloudDef {
        int speedDivider;
        int tickOffset;
        int relY;
        int width;
        int height;
    };
    static const CloudDef clouds[4] = {
        { 5,  15, -16, 48, 14 },
        { 7, 140, -26, 40, 12 },
        { 4, 270, -38, 54, 15 },
        { 6, 390, -20, 36, 11 }
    };

    const QColor cHighlight(255, 255, 255);
    const QColor cBody(248, 250, 252);
    const QColor cShade(200, 214, 230);
    const QColor cUnderbelly(168, 185, 205);

    for (int i = 0; i < 4; ++i)
    {
        const auto &cd = clouds[i];
        const int cx = minMathX - 45 + ((hudTick / cd.speedDivider) + cd.tickOffset) % mathSpan;
        const int cy = maxMathY + cd.relY;

        // Draw grand multi-tiered chunky pixel cloud domes
        for (int col = 0; col < cd.width; ++col)
        {
            // Triple overlapping dome curve profile
            const double norm = static_cast<double>(col) / cd.width;
            double domeProfile = 0.0;

            // Left dome (peaks at 25%)
            const double d1 = (norm - 0.25) / 0.25;
            if (std::abs(d1) < 1.0) domeProfile = std::max(domeProfile, 0.75 * (1.0 - d1 * d1));

            // Central grand dome (peaks at 55%)
            const double d2 = (norm - 0.55) / 0.32;
            if (std::abs(d2) < 1.0) domeProfile = std::max(domeProfile, 1.00 * (1.0 - d2 * d2));

            // Right dome (peaks at 82%)
            const double d3 = (norm - 0.82) / 0.20;
            if (std::abs(d3) < 1.0) domeProfile = std::max(domeProfile, 0.65 * (1.0 - d3 * d3));

            const int domeH = std::max(2, static_cast<int>(std::round(domeProfile * (cd.height - 1))) + 1);

            for (int r = 0; r < domeH; ++r)
            {
                const int mx = cx + col;
                const int my = cy + r;
                QColor pixCol = cBody;
                if (r >= domeH - 2)
                {
                    pixCol = cHighlight; // Bright white top crest
                }
                else if (r == 0)
                {
                    pixCol = cUnderbelly; // Atmospheric base shadow
                }
                else if (r <= 2)
                {
                    pixCol = cShade; // Soft lavender-grey shade
                }

                const int sx = grid.mathToScreenX(mx);
                const int sy = grid.mathToScreenY(my);
                painter.fillRect(sx, sy, scale, scale, pixCol);
            }
        }
    }
}

void GameRenderer::renderPixelHills(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                   int minMathX, int maxMathX, int groundMathY)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    // Soft atmospheric mountain ridges (hazier and brighter)
    const QColor farHillCol(138, 165, 186);   // Soft hazy blue mountain ridge
    const QColor nearHillCol(104, 144, 118);  // Soft atmospheric sage green
    const QColor nearHillRidge(122, 162, 136);// Ridge highlight
    const QColor pineNeedles(80, 115, 92);    // Muted spruce
    const QColor pineTrunk(95, 75, 65);

    // 1. Layer 1: Distant Rolling Blue-Grey Mountain Ridge (highest altitude in background)
    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        const int hTop = groundMathY + 22 + static_cast<int>(std::sin((mx + 40) * 0.045) * 6.0 + std::cos(mx * 0.08) * 4.0);
        for (int my = hTop; my > groundMathY + 9; --my)
        {
            const int sx = grid.mathToScreenX(mx);
            const int sy = grid.mathToScreenY(my);
            painter.fillRect(sx, sy, scale, scale, farHillCol);
        }
    }

    // 2. Layer 2: Near Hazy Sage Foothills Ridge
    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        const int hTop = groundMathY + 16 + static_cast<int>(std::sin((mx - 15) * 0.055) * 5.0 + std::sin(mx * 0.11) * 2.5);
        for (int my = hTop; my > groundMathY + 9; --my)
        {
            const QColor c = (my == hTop) ? nearHillRidge : nearHillCol;
            const int sx = grid.mathToScreenX(mx);
            const int sy = grid.mathToScreenY(my);
            painter.fillRect(sx, sy, scale, scale, c);
        }

        // Little pixel pine trees dotting the foothills ridge
        if ((mx % 16 == 0) && mx > minMathX + 6 && mx < maxMathX - 6)
        {
            const int treeY = hTop;
            const int tx = grid.mathToScreenX(mx);
            const int ty = grid.mathToScreenY(treeY + 1);
            painter.fillRect(tx, ty, scale, scale, pineTrunk);

            for (int row = 0; row < 3; ++row)
            {
                const int w = (row == 0 ? 3 : (row == 1 ? 2 : 1));
                for (int dx = -w / 2; dx <= w / 2; ++dx)
                {
                    const int px = grid.mathToScreenX(mx + dx);
                    const int py = grid.mathToScreenY(treeY + 2 + row);
                    painter.fillRect(px, py, scale, scale, pineNeedles);
                }
            }
        }
    }
}

void GameRenderer::renderPixelFarmhouse(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                       int groundMathY, int hudTick)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    // -------------------------------------------------------------
    // PERSPECTIVE VIEW: Elevated Midground Knoll & Plateau
    // The house sits on a raised pasture terrace at hy = groundMathY + 9
    // (distinctly higher than the foreground basket, but below the hills)
    // -------------------------------------------------------------
    const int knollBaseY = groundMathY + 9;
    const int hx = -42;
    const int hy = knollBaseY + 1; // Base of farmhouse resting on the elevated knoll

    auto drawPix = [&](int x, int y, const QColor &c) {
        const int sx = grid.mathToScreenX(x);
        const int sy = grid.mathToScreenY(y);
        painter.fillRect(sx, sy, scale, scale, c);
    };

    auto drawBlock = [&](int x, int y, int w, int h, const QColor &c) {
        for (int dy = 0; dy < h; ++dy)
        {
            for (int dx = 0; dx < w; ++dx)
            {
                drawPix(x + dx, y + dy, c);
            }
        }
    };

    // 1. ELEVATED MIDGROUND KNOLL & TERRACE BANKS (Stepping down to foreground)
    const QColor knollGrass(88, 134, 98);
    const QColor knollGrassHi(105, 155, 116);
    const QColor knollBank(72, 115, 82);
    const QColor knollPath(140, 125, 105);

    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);

    // Draw the elevated pasture knoll shelf across the midground
    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        // Gentle undulating knoll profile (peaks around the farmhouse)
        const int knollTop = knollBaseY + static_cast<int>(std::sin((mx + 45) * 0.04) * 2.0);
        for (int my = knollTop; my > groundMathY; --my)
        {
            QColor c = knollBank;
            if (my == knollTop)
            {
                c = knollGrassHi;
            }
            else if (my > knollTop - 3)
            {
                c = knollGrass;
            }
            drawPix(mx, my, c);
        }
    }

    // Gentle dirt path winding down from the barn towards the right
    for (int step = 0; step < 18; ++step)
    {
        const int px = hx + 10 + step;
        const int py = hy - 1 - (step / 3);
        if (py > groundMathY)
        {
            drawPix(px, py, knollPath);
            drawPix(px, py - 1, knollPath);
        }
    }

    // 2. MIDGROUND FARMHOUSE & SILO (Scaled to 75% for true perspective depth)
    // Soft, slightly muted rustic palette with atmospheric perspective
    const QColor barnRed(180, 75, 75);
    const QColor barnRedDark(148, 55, 55);
    const QColor barnTrim(240, 243, 246);
    const QColor roofCharcoal(72, 86, 98);
    const QColor roofShingle(88, 102, 115);
    const QColor doorWood(82, 58, 48);
    const QColor windowGold(255, 193, 7);
    const QColor windowAmber(255, 215, 64);
    const QColor chimneyStoneA(118, 138, 148);
    const QColor chimneyStoneB(140, 160, 170);
    const QColor chimneyCap(75, 90, 100);
    const QColor siloBodyA(162, 180, 192);
    const QColor siloBodyB(196, 210, 220);
    const QColor siloShadow(125, 145, 158);
    const QColor siloRib(95, 115, 128);
    const QColor hayGold(240, 195, 65);
    const QColor hayTie(215, 140, 35);

    // SILO (Width 6, Height 18, sitting in midground perspective)
    const int sx0 = hx + 19;
    const int sw = 6;
    for (int y = hy; y <= hy + 14; ++y)
    {
        const bool isRib = ((y - hy) % 3 == 0);
        for (int x = sx0; x < sx0 + sw; ++x)
        {
            if (isRib)
            {
                drawPix(x, y, siloRib);
            }
            else
            {
                const int col = x - sx0;
                if (col == 0) drawPix(x, y, siloShadow);
                else if (col <= 2) drawPix(x, y, siloBodyA);
                else if (col <= 4) drawPix(x, y, siloBodyB);
                else drawPix(x, y, siloShadow);
            }
        }
    }
    // Silo domed roof
    for (int dy = 0; dy < 3; ++dy)
    {
        const int domeInset = (dy == 0 ? 0 : 1);
        for (int x = sx0 + domeInset; x < sx0 + sw - domeInset; ++x)
        {
            drawPix(x, hy + 15 + dy, (dy == 2 ? siloRib : siloBodyA));
        }
    }

    // MAIN RED BARN (Width 17, Height 12)
    const int bw = 17;
    for (int y = hy; y < hy + 11; ++y)
    {
        for (int x = hx; x < hx + bw; ++x)
        {
            if ((x - hx) == 0 || (x - hx) == bw - 1)
            {
                drawPix(x, y, barnTrim); // Corner trim columns
            }
            else if ((x - hx) % 3 == 0)
            {
                drawPix(x, y, barnRedDark);
            }
            else
            {
                drawPix(x, y, barnRed);
            }
        }
    }

    // BARN DOUBLE DOORS with White "X" cross-timbers
    const int dx0 = hx + 5;
    const int dw = 6;
    const int dh = 6;
    drawBlock(dx0, hy, dw, dh, doorWood);
    for (int r = 0; r < dh; ++r)
    {
        drawPix(dx0 + r, hy + r, barnTrim);
        drawPix(dx0 + dw - 1 - r, hy + r, barnTrim);
    }
    drawBlock(dx0, hy + dh - 1, dw, 1, barnTrim);

    // UPPER LOFT GLOWING WINDOW
    const int ux = hx + 7;
    const int uy = hy + 7;
    drawBlock(ux, uy, 3, 3, barnTrim);
    drawPix(ux + 1, uy + 1, windowAmber);
    drawPix(ux + 1, uy + 2, windowGold);

    // BARN GAMBREL ROOF (Stepped charcoal shingles with white eaves)
    const int roofH = 5;
    for (int r = 0; r < roofH; ++r)
    {
        const int ry = hy + 11 + r;
        const int inset = r;
        const int rx0 = hx - 1 + inset;
        const int rx1 = hx + bw - inset;

        drawPix(rx0, ry, barnTrim);
        drawPix(rx1, ry, barnTrim);

        for (int x = rx0 + 1; x < rx1; ++x)
        {
            drawPix(x, ry, (r % 2 == 0) ? roofCharcoal : roofShingle);
        }
    }

    // Weather Vane on Roof Apex (Brass Rooster)
    const int apexX = hx + bw / 2;
    const int apexY = hy + 11 + roofH;
    drawPix(apexX, apexY, barnTrim);
    drawPix(apexX, apexY + 1, QColor(243, 156, 18));
    drawPix(apexX, apexY + 2, QColor(243, 156, 18));
    drawPix(apexX + 1, apexY + 2, QColor(230, 126, 34));

    // STONE CHIMNEY with Animated Pixel Smoke
    const int cx0 = hx - 3;
    const int cw = 2;
    const int ch = 15;
    for (int y = hy; y < hy + ch; ++y)
    {
        for (int x = cx0; x < cx0 + cw; ++x)
        {
            drawPix(x, y, ((x + y) % 2 == 0) ? chimneyStoneA : chimneyStoneB);
        }
    }
    drawBlock(cx0 - 1, hy + ch, cw + 2, 1, chimneyCap);

    // Animated chunky pixel smoke puffs drifting into the sky
    for (int puff = 0; puff < 3; ++puff)
    {
        const int offset = (hudTick + puff * 15) % 45;
        const int smkX = cx0 + 1 + (offset / 8);
        const int smkY = hy + ch + 1 + (offset / 2);
        const int smkW = 2 + (puff % 2);
        const QColor smkCol(245, 248, 252, std::max(50, 220 - offset * 4));

        for (int dy = 0; dy < 2; ++dy)
        {
            for (int dx = 0; dx < smkW; ++dx)
            {
                drawPix(smkX + dx, smkY + dy, smkCol);
            }
        }
    }

    // Golden Hay Bales beside Silo
    const int hbx = sx0 + sw + 2;
    drawBlock(hbx, hy, 3, 2, hayGold);
    drawPix(hbx + 1, hy + 1, hayTie);
    drawBlock(hbx + 4, hy, 3, 2, hayGold);
    drawPix(hbx + 5, hy + 1, hayTie);
}

void GameRenderer::renderPixelFence(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                   int minMathX, int maxMathX, int groundMathY)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    auto drawPix = [&](int x, int y, const QColor &c) {
        const int sx = grid.mathToScreenX(x);
        const int sy = grid.mathToScreenY(y);
        painter.fillRect(sx, sy, scale, scale, c);
    };

    // Rustic wooden fence along the knoll rim in the midground
    const QColor fencePost(102, 72, 60);
    const QColor fenceRail(135, 102, 90);
    const int knollBaseY = groundMathY + 9;

    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        // Don't block the barn front path (-42 to -22)
        if (mx >= -42 && mx <= -20)
        {
            continue;
        }

        const int fy = knollBaseY + static_cast<int>(std::sin((mx + 45) * 0.04) * 2.0);

        // Horizontal rails
        drawPix(mx, fy + 3, fenceRail);
        drawPix(mx, fy + 1, fenceRail);

        // Vertical posts every 8 units
        if (mx % 8 == 0)
        {
            for (int h = 0; h < 5; ++h)
            {
                drawPix(mx, fy + h, fencePost);
            }
        }
    }
}

QPixmap GameRenderer::renderFrame(const GameEngine &engine,
                                  MyGrid &grid,
                                  MyPixels &pixels,
                                  int width,
                                  int height,
                                  int scale,
                                  int hudTick)
{
    if (width <= 10 || height <= 10)
    {
        return QPixmap();
    }

    QPixmap pix(width, height);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, false);

    grid.setDimensions(width, height);
    grid.setScale(scale);

    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);
    const int minMathY = grid.screenToMathY(height);
    const int maxMathY = grid.screenToMathY(0);

    // -------------------------------------------------------------
    // BACKGROUND PASS: 100% Pure Pixelated World (Zero Smooth Drawing)
    // Brighter & Hazier Atmospheric Depth with Elevated Midground Knoll
    // -------------------------------------------------------------
    // 1. Pixelated Sky with stepped pastel daylight color bands and dithering
    renderPixelSky(painter, grid, scale, width, height, minMathX, maxMathX, minMathY, maxMathY, engine.getGroundMathY());

    // 2. Grand, puffy pixel clouds drifting across the sky
    renderPixelClouds(painter, grid, scale, width, height, minMathX, maxMathX, maxMathY, hudTick);

    // 3. Soft distant mountain ridges with evergreen tree silhouettes
    renderPixelHills(painter, grid, scale, width, height, minMathX, maxMathX, engine.getGroundMathY());

    // 4. Perspective view: Farmhouse & barn elevated on midground pasture knoll
    renderPixelFarmhouse(painter, grid, scale, width, height, engine.getGroundMathY(), hudTick);

    // 5. Midground rustic split-rail wooden fence
    renderPixelFence(painter, grid, scale, width, height, minMathX, maxMathX, engine.getGroundMathY());

    // -------------------------------------------------------------
    // FOREGROUND RASTERIZATION PASS (Crisp, Highlighted, Phosphor Glow)
    // -------------------------------------------------------------
    pixels.clear();

    // Rasterize Foreground Ground (Lush, saturated grass and rich soil)
    rasterizeGround(pixels, minMathX, maxMathX, engine.getGroundMathY(), minMathY);

    // Rasterize Birds (Enlarged 15x9 sprites with 3-frame animated wings)
    for (const auto &bird : engine.getBirds())
    {
        rasterizeBird(pixels, bird);
    }

    // Rasterize Basket (Enlarged woven wicker basket)
    rasterizeBasket(pixels, engine.getBasket());

    // Rasterize Falling Egg
    const FallingEgg &egg = engine.getFallingEgg();
    if (egg.active)
    {
        rasterizeEgg(pixels, egg);
    }

    // Rasterize Hearts in canvas top-left and level bar in top-right (when not in MENU)
    if (engine.getState() != GameState::MENU)
    {
        rasterizePixelHearts(pixels, minMathX + 3, maxMathY - 3, engine.getHearts(), 3);
        rasterizeLevelBar(pixels, maxMathX - 40, maxMathX - 3, maxMathY - 2, engine.getLevel(), engine.getScore(), hudTick);
    }

    // -------------------------------------------------------------
    // PASS 1: Pixel Glow Aura Pass (Radial glowing aura radiating beyond cells)
    // -------------------------------------------------------------
    const int glowRadius = (scale <= 3 ? 1 : (scale <= 6 ? 2 : 3));
    const int glowAlpha = (scale <= 3 ? 32 : 45);

    // Special glowing aura for Golden Egg
    if (egg.active && egg.type == EggType::GOLDEN)
    {
        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY)));
        const int auraR = scale * 7;
        QRadialGradient goldGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
        goldGlow.setColorAt(0.0, QColor(255, 215, 0, 80));
        goldGlow.setColorAt(0.4, QColor(255, 190, 0, 30));
        goldGlow.setColorAt(1.0, QColor(255, 180, 0, 0));
        painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, goldGlow);
    }
    // Special glowing aura for Basket Growth Egg
    else if (egg.active && egg.type == EggType::BASKET_GROW)
    {
        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY)));
        const int auraR = scale * 7;
        QRadialGradient growGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
        growGlow.setColorAt(0.0, QColor(46, 204, 113, 85));
        growGlow.setColorAt(0.4, QColor(39, 174, 96, 30));
        growGlow.setColorAt(1.0, QColor(39, 174, 96, 0));
        painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, growGlow);
    }
    // Special glowing radiant aura for Legendary Restoration Egg
    else if (egg.active && egg.type == EggType::BASKET_RESTORE)
    {
        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY)));
        const int auraR = scale * 8;
        QRadialGradient restoreGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
        restoreGlow.setColorAt(0.0, QColor(0, 229, 255, 95));
        restoreGlow.setColorAt(0.35, QColor(233, 30, 99, 50));
        restoreGlow.setColorAt(0.7, QColor(255, 235, 59, 25));
        restoreGlow.setColorAt(1.0, QColor(156, 39, 176, 0));
        painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, restoreGlow);
    }
    // Special glowing aura for Bomb
    else if (egg.active && egg.type == EggType::BOMB)
    {
        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY + 3.0)));
        const int sparkR = scale * 5;
        QRadialGradient sparkGlow(ex + scale / 2.0, ey + scale / 2.0, sparkR);
        sparkGlow.setColorAt(0.0, QColor(255, 120, 0, 85));
        sparkGlow.setColorAt(0.5, QColor(230, 60, 0, 30));
        sparkGlow.setColorAt(1.0, QColor(230, 50, 0, 0));
        painter.fillRect(ex + scale / 2 - sparkR, ey + scale / 2 - sparkR, sparkR * 2, sparkR * 2, sparkGlow);
    }

    // Glowing aura for all active raster pixels
    for (const auto &entry : pixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        const int sx = grid.mathToScreenX(x);
        const int sy = grid.mathToScreenY(y);

        if (sx + scale + glowRadius < 0 || sx - glowRadius >= width ||
            sy + scale + glowRadius < 0 || sy - glowRadius >= height)
        {
            continue;
        }

        QColor glowColor = entry.second;
        glowColor.setAlpha(glowAlpha);
        painter.fillRect(sx - glowRadius, sy - glowRadius, scale + 2 * glowRadius, scale + 2 * glowRadius, glowColor);
    }

    // Glowing aura for particles
    const auto &particles = engine.getParticles();
    for (const auto &p : particles)
    {
        const int sx = grid.mathToScreenX(static_cast<int>(std::round(p.x)));
        const int sy = grid.mathToScreenY(static_cast<int>(std::round(p.y)));
        const int pGlowR = glowRadius + 1;
        QColor pGlowColor = p.color;
        pGlowColor.setAlpha(65);
        painter.fillRect(sx - pGlowR, sy - pGlowR, scale + 2 * pGlowR, scale + 2 * pGlowR, pGlowColor);
    }

    // -------------------------------------------------------------
    // PASS 2: Pixel Core Pass (Solid crisp cells with phosphor highlight)
    // -------------------------------------------------------------
    for (const auto &entry : pixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        paintCell(painter, grid, scale, width, height, x, y, entry.second);
    }

    // Dynamic particles core pass
    for (const auto &p : particles)
    {
        paintCell(painter, grid, scale, width, height,
                  static_cast<int>(std::round(p.x)),
                  static_cast<int>(std::round(p.y)),
                  p.color);
    }

    // -------------------------------------------------------------
    // FLOATING TEXTS & HUD LABELS
    // -------------------------------------------------------------
    QFont font("Segoe UI", 12, QFont::Bold);
    painter.setFont(font);
    for (const auto &t : engine.getFloatingTexts())
    {
        const int sx = grid.mathToScreenX(static_cast<int>(std::round(t.x)));
        const int sy = grid.mathToScreenY(static_cast<int>(std::round(t.y)));

        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(sx - 39, sy + 1, t.text);
        painter.setPen(t.color);
        painter.drawText(sx - 40, sy, t.text);
    }

    // Draw HUD text and Level Bar text
    const int level = engine.getLevel();
    const int score = engine.getScore();
    if (engine.getState() == GameState::PLAYING || engine.getState() == GameState::PAUSED)
    {
        // 1. Draw Level, Score, and High Score directly below the three hearts (top-left)
        const int heartsStartX = grid.mathToScreenX(minMathX + 3);
        const int heartsBottomY = grid.mathToScreenY(maxMathY - 7);
        const int hudLeftX = heartsStartX;
        const int hudStartY = heartsBottomY + 28; // Lowered as requested

        // Level Number
        QFont hudLevelFont("Segoe UI", 11, QFont::Bold);
        painter.setFont(hudLevelFont);
        QString levelStr = QString("LEVEL %1").arg(level);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 1, levelStr);
        painter.setPen(QColor(199, 125, 255)); // Lavender purple
        painter.drawText(hudLeftX, hudStartY, levelStr);

        // Current Score
        QFont hudScoreFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(hudScoreFont);
        QString scoreStr = QString("SCORE: %1").arg(score);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 18, scoreStr);
        painter.setPen(QColor(6, 214, 160)); // Emerald green
        painter.drawText(hudLeftX, hudStartY + 17, scoreStr);

        // Best Score
        QString bestStr = QString("BEST: %1").arg(engine.getHighScore());
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 35, bestStr);
        painter.setPen(QColor(255, 183, 3)); // Warm gold
        painter.drawText(hudLeftX, hudStartY + 34, bestStr);

        // 2. Draw Progress Bar Text below the bar (top-right)
        const bool maxed = level >= GameEngine::MAX_LEVEL;
        const int levelStart = (level - 1) * GameEngine::POINTS_PER_LEVEL;
        const int earned = maxed ? GameEngine::POINTS_PER_LEVEL : std::clamp(score - levelStart, 0, GameEngine::POINTS_PER_LEVEL);
        const int remaining = GameEngine::POINTS_PER_LEVEL - earned;
        const QString progressLabel = maxed ? QString("MAX LEVEL (50)")
                                            : QString("%1 PTS TO NEXT LEVEL").arg(remaining);

        QFont progressFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(progressFont);

        const int sx = grid.mathToScreenX(maxMathX - 40);
        const int sy = grid.mathToScreenY(maxMathY - 8);
        const int barW = grid.mathToScreenX(maxMathX - 3) - sx;

        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(QRect(sx + 1, sy + 1, barW, 20), Qt::AlignCenter, progressLabel);

        painter.setPen(maxed ? QColor(255, 215, 0) : QColor(224, 230, 237));
        painter.drawText(QRect(sx, sy, barW, 20), Qt::AlignCenter, progressLabel);
    }

    // Level-up banner (fades out)
    const int bannerTicks = engine.getLevelBannerTicks();
    if (bannerTicks > 0 && engine.getState() == GameState::PLAYING)
    {
        const int alpha = std::min(255, bannerTicks * 8);
        painter.setPen(QColor(0, 0, 0, alpha * 2 / 3));
        QFont bannerFont("Segoe UI", 34, QFont::Black);
        painter.setFont(bannerFont);
        const QRect bannerRect(0, height / 3 - 40, width, 70);
        painter.drawText(bannerRect.translated(3, 3), Qt::AlignCenter, QString("LEVEL %1").arg(level));
        painter.setPen(QColor(199, 125, 255, alpha));
        painter.drawText(bannerRect, Qt::AlignCenter, QString("LEVEL %1").arg(level));

        QFont subFont("Segoe UI", 13, QFont::DemiBold);
        painter.setFont(subFont);
        painter.setPen(QColor(224, 230, 237, alpha));
        painter.drawText(QRect(0, height / 3 + 25, width, 30), Qt::AlignCenter,
                         "Faster eggs • More frequent drops • Smaller basket");
    }

    // Overlays for Start Menu, Game Over, and Paused
    if (engine.getState() == GameState::MENU)
    {
        // 1. Semi-transparent dark atmospheric backdrop over animated pixel sky
        painter.fillRect(0, 0, width, height, QColor(10, 13, 20, 228));

        // 2. Card Dimensions (smaller as per request)
        const int cardW = std::min(width - 40, 500);
        const int cardH = std::min(height - 40, 360);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        // Card Frame
        painter.setPen(QPen(QColor(48, 58, 80), 2));
        painter.setBrush(QColor(20, 24, 36, 248));
        painter.drawRoundedRect(cardX, cardY, cardW, cardH, 12, 12);

        // Header Accent line
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 209, 102));
        painter.drawRoundedRect(cardX + 24, cardY + 70, cardW - 48, 2, 1, 1);

        // Title: EGGSCELLENT CATCH
        QFont titleFont("Segoe UI", 26, QFont::Black);
        painter.setFont(titleFont);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(QRect(cardX + 2, cardY + 16, cardW, 40), Qt::AlignCenter, "EGGSCELLENT CATCH");
        painter.setPen(QColor(255, 209, 102));
        painter.drawText(QRect(cardX, cardY + 14, cardW, 40), Qt::AlignCenter, "EGGSCELLENT CATCH");

        QFont subFont("Segoe UI", 11, QFont::DemiBold);
        painter.setFont(subFont);
        painter.setPen(QColor(168, 199, 250));
        painter.drawText(QRect(cardX, cardY + 50, cardW, 18), Qt::AlignCenter, "FARM FRENZY EDITION"); // Changed theme subtitle

        // Single Column Content based on slide index
        const int colX = cardX + 30;
        int y = cardY + 94;

        QFont secFont("Segoe UI", 12, QFont::Bold);
        QFont bodyFont("Segoe UI", 11, QFont::Normal);
        QFont boldBodyFont("Segoe UI", 11, QFont::Bold);

        int slideIndex = engine.getMenuSlideIndex();

        if (slideIndex == 0)
        {
            // --- Slide 0: HOW TO PLAY ---
            painter.setFont(secFont);
            painter.setPen(QColor(6, 214, 160)); // Emerald
            painter.drawText(colX, y, "HOW TO PLAY");
            y += 28;

            painter.setFont(bodyFont);
            painter.setPen(QColor(224, 230, 237));
            painter.drawText(colX, y, "• Catch falling eggs before they hit the ground!");
            y += 24;
            painter.drawText(colX, y, "• 3 missed eggs = Game Over (lose hearts).");
            y += 24;
            painter.drawText(colX, y, "• Avoid bombs! Catching a bomb is fatal.");
            y += 24;
            painter.drawText(colX, y, "• Level up every 100 points:");
            y += 24;
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(199, 125, 255));
            painter.drawText(colX + 16, y, "Eggs fall faster & basket shrinks!");
        }
        else if (slideIndex == 1)
        {
            // --- Slide 1: CONTROLS ---
            painter.setFont(secFont);
            painter.setPen(QColor(199, 125, 255)); // Purple
            painter.drawText(colX, y, "CONTROLS");
            y += 28;

            painter.setFont(bodyFont);
            painter.setPen(QColor(224, 230, 237));
            painter.drawText(colX, y, "• Move Basket : Mouse or [A / D] / [Left / Right]");
            y += 24;
            painter.drawText(colX, y, "• Next Slide in Menu : [→] Right Arrow Key");
            y += 24;
            painter.drawText(colX, y, "• Start Game : [SPACE] or [ENTER]");
            y += 24;
            painter.drawText(colX, y, "• Pause / Resume : [SPACE]");
            y += 24;
            painter.drawText(colX, y, "• Restore / Restart : [R]");
        }
        else if (slideIndex == 2)
        {
            // --- Slide 2: POINTS PER EGG ---
            painter.setFont(secFont);
            painter.setPen(QColor(255, 183, 3)); // Gold
            painter.drawText(colX, y, "POINTS PER EGG");
            y += 24;

            int lineH = 22;
            
            // Regular Egg
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(255, 255, 255));
            painter.drawText(colX, y, "[O] Regular");
            painter.setPen(QColor(6, 214, 160));
            painter.drawText(colX + 120, y, "+10 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 190, y, "Standard egg");
            y += lineH;

            // Golden Egg
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(255, 215, 0));
            painter.drawText(colX, y, "[*] Golden");
            painter.setPen(QColor(255, 215, 0));
            painter.drawText(colX + 120, y, "+50 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 190, y, "Rare bonus");
            y += lineH;

            // Bomb
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(239, 71, 111));
            painter.drawText(colX, y, "[X] Bomb");
            painter.setPen(QColor(239, 71, 111));
            painter.drawText(colX + 120, y, "FATAL");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 190, y, "Instant Game Over!");
            y += lineH;

            // Growth Egg
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(46, 204, 113));
            painter.drawText(colX, y, "[+] Growth");
            painter.setPen(QColor(46, 204, 113));
            painter.drawText(colX + 120, y, "+25 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 190, y, "Widens basket by +2");
            y += lineH;

            // Restore Egg
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(0, 229, 255));
            painter.drawText(colX, y, "[^] Restore");
            painter.setPen(QColor(0, 229, 255));
            painter.drawText(colX + 120, y, "+100 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 190, y, "Restores basket to max");
        }

        // Bottom Navigation and Start Hints
        QFont navFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(navFont);
        painter.setPen(QColor(255, 209, 102));
        QString slideIndicator = QString("Page %1 of 3   •   Press [→] to see next").arg(slideIndex + 1);
        painter.drawText(QRect(cardX, cardY + cardH - 60, cardW, 22), Qt::AlignCenter, slideIndicator);

        QFont hintFont("Segoe UI", 12, QFont::Bold);
        painter.setFont(hintFont);
        painter.setPen(QColor(6, 214, 160));
        painter.drawText(QRect(cardX, cardY + cardH - 34, cardW, 24), Qt::AlignCenter,
                         "Press [SPACE] or [ENTER] to Start");
    }
    else if (engine.getState() == GameState::GAME_OVER)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 215));

        painter.setPen(QColor(239, 71, 111));
        QFont titleFont("Segoe UI", 32, QFont::Bold);
        painter.setFont(titleFont);
        painter.drawText(QRect(0, height / 2 - 110, width, 60), Qt::AlignCenter, "GAME OVER");

        painter.setPen(QColor(255, 209, 102));
        QFont subFont("Segoe UI", 16, QFont::DemiBold);
        painter.setFont(subFont);
        painter.drawText(QRect(0, height / 2 - 40, width, 40), Qt::AlignCenter, engine.getGameOverReason());

        painter.setPen(QColor(6, 214, 160));
        QFont scoreFont("Segoe UI", 15, QFont::Normal);
        painter.setFont(scoreFont);
        painter.drawText(QRect(0, height / 2 + 10, width, 30), Qt::AlignCenter,
                         QString("Final Score: %1    |    Level Reached: %2    |    High Score: %3")
                             .arg(score).arg(level).arg(engine.getHighScore()));

        painter.setPen(QColor(168, 199, 250));
        QFont promptFont("Segoe UI", 13, QFont::Normal);
        painter.setFont(promptFont);
        painter.drawText(QRect(0, height / 2 + 60, width, 30), Qt::AlignCenter,
                         "Press [R] to restore / restart or [SPACE] to play again");
    }
    else if (engine.getState() == GameState::PAUSED)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 180));

        painter.setPen(QColor(255, 209, 102));
        QFont pauseFont("Segoe UI", 28, QFont::Bold);
        painter.setFont(pauseFont);
        painter.drawText(QRect(0, height / 2 - 50, width, 50), Qt::AlignCenter, "PAUSED");

        painter.setPen(QColor(224, 230, 237));
        QFont promptFont("Segoe UI", 13, QFont::Normal);
        painter.setFont(promptFont);
        painter.drawText(QRect(0, height / 2 + 15, width, 30), Qt::AlignCenter,
                         "Press [SPACE] to resume or [R] to restore / restart");
    }

    painter.end();
    return pix;
}
