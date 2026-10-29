#include "GameRenderer.h"
#include "GameEngine.h"
#include "AudioManager.h"
#include "Grid.h"
#include "Pixel.h"
#include "PixelFont.h"
#include "Basket.h"
#include "Bird.h"
#include "Boss.h"
#include "FallingEgg.h"
#include "GameTypes.h"
#include "Particle.h"
#include <QRadialGradient>
#include <QLinearGradient>
#include <QFont>
#include <QRect>
#include <algorithm>
#include <cmath>

static QColor lerpColor(const QColor &a, const QColor &b, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return QColor(
        static_cast<int>(std::round(a.red() + (b.red() - a.red()) * t)),
        static_cast<int>(std::round(a.green() + (b.green() - a.green()) * t)),
        static_cast<int>(std::round(a.blue() + (b.blue() - a.blue()) * t))
    );
}

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
    // Solid raster cell
    painter.fillRect(sx, sy, scale, scale, color);

    // Inner phosphor highlight
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
        pixels.setPixel(x, gY, ((x % 3 == 0) ? QColor(88, 214, 141) : QColor(46, 204, 113)));
        pixels.setPixel(x, gY - 1, QColor(39, 174, 96));

        for (int y = gY - 2; y >= minY; --y)
        {
            if ((x * 7 + y * 13) % 11 == 0)
            {
                pixels.setPixel(x, y, QColor(141, 110, 99));
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
    for (int h = 0; h < maxHearts; ++h)
    {
        const int hx = startX + h * 7;
        const int hy = startY;
        const bool active = (h < currentHearts);
        const QColor heartCol = active ? QColor(239, 71, 111) : QColor(70, 78, 96);

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
    const int levelStart = GameEngine::levelStartScore(level);
    const int levelLen = GameEngine::levelLength(level);
    const int earned = maxed ? levelLen : std::clamp(score - levelStart, 0, levelLen);

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
    const int filledW = (innerW * earned) / levelLen;
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

            if (x == shimmerX || x == shimmerX - 1)
            {
                c = QColor(255, 255, 255);
            }

            pixels.setPixel(x, topY - 1, c.lighter(120));
            pixels.setPixel(x, topY - 2, c);
            pixels.setPixel(x, topY - 3, c.darker(125));
        }
        else
        {
            const bool isNotch = ((i + 1) % (innerW / 4) == 0);
            const QColor c = isNotch ? notchCol : emptyCol;
            pixels.setPixel(x, topY - 1, c);
            pixels.setPixel(x, topY - 2, c);
            pixels.setPixel(x, topY - 3, c.darker(115));
        }
    }
}

void GameRenderer::rasterizeEggStatus(MyPixels &pixels, int startX, int topY, int count, bool isHardMode, int hudTick)
{
    Q_UNUSED(hudTick);
    // 5x6 compact Raster Pixel Green Growth Egg (Pure pixel art icon with phosphor glow)
    const QColor contour(20, 90, 50);
    const QColor shellLight(46, 204, 113);
    const QColor shellMid(39, 174, 96);
    const QColor glint(255, 255, 240);

    static const int eggSprite[6][5] = {
        { 0, 1, 1, 1, 0 },
        { 1, 2, 4, 2, 1 },
        { 1, 2, 2, 3, 1 },
        { 1, 2, 3, 3, 1 },
        { 1, 3, 3, 3, 1 },
        { 0, 1, 1, 1, 0 }
    };

    for (int r = 0; r < 6; ++r)
    {
        for (int c = 0; c < 5; ++c)
        {
            const int val = eggSprite[r][c];
            if (val == 0) continue;
            QColor col = (val == 1) ? contour : ((val == 2) ? shellLight : ((val == 3) ? shellMid : glint));
            pixels.setPixel(startX + c, topY - r, col);
        }
    }

    // Compact count and keybinds
    QString text = QString("x%1 [H]").arg(count);
    if (isHardMode)
    {
        text += " [C]";
    }
    const QColor textCol = (count > 0) ? QColor(46, 204, 113) : QColor(140, 160, 150);
    PixelFont::drawText(pixels, startX + 7, topY - 1, text, textCol);
}

void GameRenderer::renderPixelSky(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                  int minMathX, int maxMathX, int minMathY, int maxMathY, int groundMathY,
                                  double dayNightProgress, double nightRatio, bool isStorm, int flashTicks, int hudTick)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(minMathY);

    static const QColor dayBands[10] = {
        QColor(92, 138, 186),
        QColor(112, 154, 198),
        QColor(133, 172, 210),
        QColor(154, 188, 220),
        QColor(176, 204, 229),
        QColor(196, 218, 237),
        QColor(214, 229, 243),
        QColor(228, 237, 246),
        QColor(241, 238, 228),
        QColor(246, 233, 218)
    };

    static const QColor sunsetBands[10] = {
        QColor(58, 42, 88),
        QColor(76, 48, 102),
        QColor(112, 54, 116),
        QColor(154, 62, 118),
        QColor(192, 70, 105),
        QColor(224, 86, 88),
        QColor(242, 114, 72),
        QColor(250, 146, 60),
        QColor(254, 182, 62),
        QColor(255, 212, 118)
    };

    static const QColor nightBands[10] = {
        QColor(8, 11, 20),
        QColor(11, 15, 28),
        QColor(14, 20, 36),
        QColor(18, 26, 46),
        QColor(22, 32, 56),
        QColor(26, 38, 66),
        QColor(30, 44, 76),
        QColor(35, 50, 84),
        QColor(42, 58, 94),
        QColor(50, 68, 105)
    };

    static const QColor dawnBands[10] = {
        QColor(56, 70, 115),
        QColor(76, 90, 138),
        QColor(105, 118, 162),
        QColor(138, 145, 185),
        QColor(172, 168, 202),
        QColor(204, 186, 208),
        QColor(228, 196, 202),
        QColor(244, 206, 194),
        QColor(252, 218, 188),
        QColor(255, 234, 198)
    };

    static const QColor stormBands[10] = {
        QColor(22, 24, 34),
        QColor(28, 30, 42),
        QColor(34, 36, 50),
        QColor(40, 43, 58),
        QColor(46, 50, 66),
        QColor(52, 56, 74),
        QColor(58, 62, 80),
        QColor(64, 68, 86),
        QColor(70, 74, 92),
        QColor(78, 82, 98)
    };

    const double dawnEnd = 0.12;
    const double nightStart = 1.0 - nightRatio;
    const double sunsetStart = nightStart - 0.14;

    double p = dayNightProgress;
    QColor currentSkyBands[10];

    for (int b = 0; b < 10; ++b)
    {
        QColor bandCol;
        if (p < dawnEnd)
        {
            const double t = p / dawnEnd;
            bandCol = lerpColor(dawnBands[b], dayBands[b], t);
        }
        else if (p < sunsetStart)
        {
            bandCol = dayBands[b];
        }
        else if (p < nightStart)
        {
            const double t = (p - sunsetStart) / (nightStart - sunsetStart);
            if (t < 0.5)
            {
                bandCol = lerpColor(dayBands[b], sunsetBands[b], t * 2.0);
            }
            else
            {
                bandCol = lerpColor(sunsetBands[b], nightBands[b], (t - 0.5) * 2.0);
            }
        }
        else
        {
            const double t = (p - nightStart) / nightRatio;
            if (t > 0.85)
            {
                bandCol = lerpColor(nightBands[b], dawnBands[b], (t - 0.85) / 0.15);
            }
            else
            {
                bandCol = nightBands[b];
            }
        }

        if (isStorm)
        {
            bandCol = lerpColor(bandCol, stormBands[b], 0.75);
        }

        if (flashTicks > 0)
        {
            bandCol = lerpColor(bandCol, QColor(245, 250, 255), 0.85);
        }

        currentSkyBands[b] = bandCol;
    }

    const int bandCount = 10;
    const int bandH = std::max(4, (maxMathY - groundMathY) / bandCount);

    for (int b = 0; b < bandCount; ++b)
    {
        const int bandTop = maxMathY - b * bandH;
        const int bandBottom = (b == bandCount - 1) ? (groundMathY + 1) : (bandTop - bandH + 1);
        const QColor bandCol = currentSkyBands[b];
        const QColor nextCol = (b + 1 < bandCount) ? currentSkyBands[b + 1] : bandCol;

        for (int my = bandTop; my >= bandBottom; --my)
        {
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

    // Stars & Moon at Night / Twilight
    const bool isNightOrTwilight = (p >= sunsetStart + 0.05 || p < dawnEnd * 0.5);
    if (isNightOrTwilight && !isStorm)
    {
        double starVisibility = 1.0;
        if (p >= sunsetStart && p < nightStart)
        {
            starVisibility = (p - sunsetStart) / (nightStart - sunsetStart);
        }
        else if (p < dawnEnd)
        {
            starVisibility = 1.0 - (p / (dawnEnd * 0.5));
        }

        if (starVisibility > 0.15)
        {
            const int starAlpha = static_cast<int>(255 * starVisibility);
            const int totalSpanX = std::max(60, maxMathX - minMathX);
            const int totalSpanY = std::max(20, maxMathY - (groundMathY + 16));

            for (int s = 0; s < 38; ++s)
            {
                const int hashX = ((s * 107 + 37) % totalSpanX);
                const int hashY = ((s * 61 + 19) % totalSpanY);
                const int starX = minMathX + hashX;
                const int starY = groundMathY + 18 + hashY;

                const bool twinkle = ((hudTick / 4 + s * 7) % 6 == 0);
                const QColor starCol = twinkle ? QColor(255, 255, 255, starAlpha)
                                               : QColor(200, 220, 255, starAlpha * 2 / 3);

                const int sx = grid.mathToScreenX(starX);
                const int sy = grid.mathToScreenY(starY);
                painter.fillRect(sx, sy, scale, scale, starCol);
            }

            const int moonX = minMathX + static_cast<int>(totalSpanX * 0.78);
            const int moonY = maxMathY - 14;

            const int mx = grid.mathToScreenX(moonX);
            const int my = grid.mathToScreenY(moonY);
            const int mR = scale * 8;
            QRadialGradient moonHalo(mx + scale / 2.0, my + scale / 2.0, mR);
            moonHalo.setColorAt(0.0, QColor(255, 255, 230, static_cast<int>(60 * starVisibility)));
            moonHalo.setColorAt(0.5, QColor(210, 230, 255, static_cast<int>(25 * starVisibility)));
            moonHalo.setColorAt(1.0, QColor(200, 220, 255, 0));
            painter.fillRect(mx + scale / 2 - mR, my + scale / 2 - mR, mR * 2, mR * 2, moonHalo);

            static const int moonPixels[5][5] = {
                { 0, 1, 1, 1, 0 },
                { 1, 1, 2, 0, 0 },
                { 1, 1, 2, 0, 0 },
                { 1, 1, 2, 0, 0 },
                { 0, 1, 1, 1, 0 }
            };
            for (int r = 0; r < 5; ++r)
            {
                for (int c = 0; c < 5; ++c)
                {
                    if (moonPixels[r][c] == 1)
                    {
                        const int px = grid.mathToScreenX(moonX + c - 2);
                        const int py = grid.mathToScreenY(moonY - r + 2);
                        painter.fillRect(px, py, scale, scale, QColor(255, 254, 235, starAlpha));
                    }
                    else if (moonPixels[r][c] == 2)
                    {
                        const int px = grid.mathToScreenX(moonX + c - 2);
                        const int py = grid.mathToScreenY(moonY - r + 2);
                        painter.fillRect(px, py, scale, scale, QColor(220, 228, 240, starAlpha));
                    }
                }
            }
        }
    }
}

void GameRenderer::renderPixelClouds(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                    int minMathX, int maxMathX, int maxMathY, int cloudTick,
                                    double dayNightProgress, double nightRatio, bool isStorm)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    const int mathSpan = maxMathX - minMathX + 90;

    const double nightStart = 1.0 - nightRatio;
    const double sunsetStart = nightStart - 0.14;
    double nightFactor = 0.0;
    double sunsetFactor = 0.0;

    if (dayNightProgress >= sunsetStart && dayNightProgress < nightStart)
    {
        sunsetFactor = (dayNightProgress - sunsetStart) / 0.14;
    }
    else if (dayNightProgress >= nightStart || dayNightProgress < 0.06)
    {
        nightFactor = 1.0;
    }

    QColor cHighlight = lerpColor(QColor(255, 255, 255), QColor(255, 180, 140), sunsetFactor);
    cHighlight = lerpColor(cHighlight, QColor(70, 82, 105), nightFactor);

    QColor cBody = lerpColor(QColor(248, 250, 252), QColor(230, 140, 130), sunsetFactor);
    cBody = lerpColor(cBody, QColor(50, 60, 80), nightFactor);

    QColor cShade = lerpColor(QColor(200, 214, 230), QColor(160, 90, 110), sunsetFactor);
    cShade = lerpColor(cShade, QColor(35, 42, 60), nightFactor);

    QColor cUnderbelly = lerpColor(QColor(168, 185, 205), QColor(120, 60, 90), sunsetFactor);
    cUnderbelly = lerpColor(cUnderbelly, QColor(24, 30, 45), nightFactor);

    if (isStorm)
    {
        cHighlight = QColor(68, 72, 88);
        cBody = QColor(48, 52, 68);
        cShade = QColor(36, 40, 54);
        cUnderbelly = QColor(26, 28, 40);
    }

    for (int i = 0; i < NUM_CLOUD_BANKS; ++i)
    {
        const auto &cd = getCloudDef(i);
        const int cx = minMathX - 45 + ((cloudTick / cd.speedDivider) + cd.tickOffset) % mathSpan;
        const int cy = maxMathY + cd.relY;

        for (int col = 0; col < cd.width; ++col)
        {
            const double norm = static_cast<double>(col) / cd.width;
            double domeProfile = 0.0;

            const double d1 = (norm - 0.25) / 0.25;
            if (std::abs(d1) < 1.0) domeProfile = std::max(domeProfile, 0.75 * (1.0 - d1 * d1));

            const double d2 = (norm - 0.55) / 0.32;
            if (std::abs(d2) < 1.0) domeProfile = std::max(domeProfile, 1.00 * (1.0 - d2 * d2));

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
                    pixCol = cHighlight;
                }
                else if (r == 0)
                {
                    pixCol = cUnderbelly;
                }
                else if (r <= 2)
                {
                    pixCol = cShade;
                }

                const int sx = grid.mathToScreenX(mx);
                const int sy = grid.mathToScreenY(my);
                painter.fillRect(sx, sy, scale, scale, pixCol);
            }
        }
    }
}

void GameRenderer::renderPixelHills(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                   int minMathX, int maxMathX, int groundMathY,
                                   double dayNightProgress, double nightRatio, bool isStorm)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    const double nightStart = 1.0 - nightRatio;
    const double sunsetStart = nightStart - 0.14;
    double nightFactor = 0.0;
    if (dayNightProgress >= nightStart || dayNightProgress < 0.06)
    {
        nightFactor = 1.0;
    }
    else if (dayNightProgress >= sunsetStart)
    {
        nightFactor = (dayNightProgress - sunsetStart) / 0.14;
    }

    QColor ridgeA = lerpColor(QColor(140, 168, 192), QColor(25, 32, 52), nightFactor);
    QColor ridgeB = lerpColor(QColor(165, 190, 210), QColor(18, 24, 40), nightFactor);
    QColor hillA = lerpColor(QColor(95, 142, 115), QColor(20, 38, 30), nightFactor);
    QColor hillB = lerpColor(QColor(118, 168, 138), QColor(14, 28, 22), nightFactor);
    QColor pineTrunk = lerpColor(QColor(88, 62, 52), QColor(28, 20, 18), nightFactor);
    QColor pineNeedles = lerpColor(QColor(52, 98, 72), QColor(15, 32, 24), nightFactor);

    if (isStorm)
    {
        ridgeA = QColor(32, 36, 48);
        ridgeB = QColor(24, 28, 38);
        hillA = QColor(28, 38, 34);
        hillB = QColor(20, 30, 26);
    }

    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        // 1. Distant Ridges: fills all the way down to groundMathY so ZERO sky gaps occur!
        const int rH = groundMathY + 28 + static_cast<int>(std::sin(mx * 0.025 + 0.8) * 8.0 + std::cos(mx * 0.05) * 3.0);
        for (int my = rH; my >= groundMathY; --my)
        {
            const QColor c = (my == rH) ? ridgeB : ridgeA;
            const int sx = grid.mathToScreenX(mx);
            const int sy = grid.mathToScreenY(my);
            painter.fillRect(sx, sy, scale, scale, c);
        }

        // 2. Near Pine Hills: fills seamlessly down to groundMathY
        const int hTop = groundMathY + 17 + static_cast<int>(std::sin(mx * 0.045 + 2.1) * 6.0 + std::cos(mx * 0.02) * 4.0);
        for (int my = hTop; my >= groundMathY; --my)
        {
            const QColor c = (my == hTop) ? hillB : hillA;
            const int sx = grid.mathToScreenX(mx);
            const int sy = grid.mathToScreenY(my);
            painter.fillRect(sx, sy, scale, scale, c);
        }

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
                                       int groundMathY, int hudTick,
                                       double dayNightProgress, double nightRatio, bool isStorm)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    const int knollBaseY = groundMathY + 9;
    const int hx = -42;
    const int hy = knollBaseY + 1;

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

    const double nightStart = 1.0 - nightRatio;
    const double sunsetStart = nightStart - 0.14;
    const bool isDark = (dayNightProgress >= sunsetStart || dayNightProgress < 0.08 || isStorm);

    const QColor knollGrass = isDark ? QColor(38, 55, 42) : QColor(88, 134, 98);
    const QColor knollGrassHi = isDark ? QColor(48, 70, 52) : QColor(105, 155, 116);
    const QColor knollBank = isDark ? QColor(28, 42, 32) : QColor(72, 115, 82);
    const QColor knollPath = isDark ? QColor(55, 50, 42) : QColor(140, 125, 105);

    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);

    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        const int knollTop = knollBaseY + static_cast<int>(std::sin((mx + 45) * 0.04) * 2.0);
        for (int my = knollTop; my > groundMathY; --my)
        {
            QColor c = knollBank;
            if (my == knollTop) c = knollGrassHi;
            else if (my > knollTop - 3) c = knollGrass;
            drawPix(mx, my, c);
        }
    }

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

    const QColor barnRed = isDark ? QColor(95, 36, 36) : QColor(180, 75, 75);
    const QColor barnRedDark = isDark ? QColor(75, 26, 26) : QColor(148, 55, 55);
    const QColor barnTrim = isDark ? QColor(140, 145, 150) : QColor(240, 243, 246);
    const QColor roofCharcoal = isDark ? QColor(36, 42, 50) : QColor(72, 86, 98);
    const QColor roofShingle = isDark ? QColor(44, 52, 60) : QColor(88, 102, 115);
    const QColor doorWood = isDark ? QColor(42, 30, 24) : QColor(82, 58, 48);

    const QColor windowGold = isDark ? QColor(255, 214, 10) : QColor(255, 193, 7);
    const QColor windowAmber = isDark ? QColor(255, 235, 100) : QColor(255, 215, 64);

    const QColor chimneyStoneA = isDark ? QColor(60, 70, 78) : QColor(118, 138, 148);
    const QColor chimneyStoneB = isDark ? QColor(75, 85, 95) : QColor(140, 160, 170);
    const QColor chimneyCap = isDark ? QColor(45, 52, 60) : QColor(75, 90, 100);
    const QColor siloBodyA = isDark ? QColor(75, 88, 98) : QColor(162, 180, 192);
    const QColor siloBodyB = isDark ? QColor(95, 110, 120) : QColor(196, 210, 220);
    const QColor siloShadow = isDark ? QColor(55, 65, 75) : QColor(125, 145, 158);
    const QColor siloRib = isDark ? QColor(45, 55, 65) : QColor(95, 115, 128);

    // SILO
    const int sx0 = hx + 19;
    const int sw = 6;
    for (int y = hy; y <= hy + 14; ++y)
    {
        const bool isRib = ((y - hy) % 3 == 0);
        for (int x = sx0; x < sx0 + sw; ++x)
        {
            if (isRib) drawPix(x, y, siloRib);
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
    for (int dy = 0; dy < 3; ++dy)
    {
        const int domeInset = (dy == 0 ? 0 : 1);
        for (int x = sx0 + domeInset; x < sx0 + sw - domeInset; ++x)
        {
            drawPix(x, hy + 15 + dy, (dy == 2 ? siloRib : siloBodyA));
        }
    }

    // MAIN BARN
    const int bw = 17;
    for (int y = hy; y < hy + 11; ++y)
    {
        for (int x = hx; x < hx + bw; ++x)
        {
            if ((x - hx) == 0 || (x - hx) == bw - 1) drawPix(x, y, barnTrim);
            else if ((x - hx) % 3 == 0) drawPix(x, y, barnRedDark);
            else drawPix(x, y, barnRed);
        }
    }

    // BARN DOORS
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

    // GAMBREL ROOF
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

    // Weather Vane
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

    for (int puff = 0; puff < 3; ++puff)
    {
        const int offset = (hudTick + puff * 15) % 45;
        const int smkX = cx0 + 1 + (offset / 8);
        const int smkY = hy + ch + 1 + (offset / 2);
        const int smkW = 2 + (puff % 2);
        const QColor smkCol(245, 248, 252, std::max(40, 220 - offset * 4));
        for (int dy = 0; dy < 2; ++dy)
        {
            for (int dx = 0; dx < smkW; ++dx)
            {
                drawPix(smkX + dx, smkY + dy, smkCol);
            }
        }
    }

    // Hay Bales
    const int hbx = sx0 + sw + 2;
    drawBlock(hbx, hy, 3, 2, isDark ? QColor(140, 105, 35) : QColor(240, 195, 65));
    drawPix(hbx + 1, hy + 1, isDark ? QColor(110, 75, 20) : QColor(215, 140, 35));
    drawBlock(hbx + 4, hy, 3, 2, isDark ? QColor(140, 105, 35) : QColor(240, 195, 65));
    drawPix(hbx + 5, hy + 1, isDark ? QColor(110, 75, 20) : QColor(215, 140, 35));
}

void GameRenderer::renderPixelFence(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                   int minMathX, int maxMathX, int groundMathY,
                                   double dayNightProgress, double nightRatio, bool isStorm)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    auto drawPix = [&](int x, int y, const QColor &c) {
        const int sx = grid.mathToScreenX(x);
        const int sy = grid.mathToScreenY(y);
        painter.fillRect(sx, sy, scale, scale, c);
    };

    const double nightStart = 1.0 - nightRatio;
    const bool isDark = (dayNightProgress >= nightStart || dayNightProgress < 0.06 || isStorm);

    const QColor fencePost = isDark ? QColor(48, 34, 28) : QColor(102, 72, 60);
    const QColor fenceRail = isDark ? QColor(64, 46, 38) : QColor(135, 102, 90);
    const int knollBaseY = groundMathY + 9;

    for (int mx = minMathX; mx <= maxMathX; ++mx)
    {
        if (mx >= -42 && mx <= -20)
        {
            continue;
        }

        const int fy = knollBaseY + static_cast<int>(std::sin((mx + 45) * 0.04) * 2.0);
        drawPix(mx, fy + 3, fenceRail);
        drawPix(mx, fy + 1, fenceRail);

        if (mx % 8 == 0)
        {
            for (int h = 0; h < 5; ++h)
            {
                drawPix(mx, fy + h, fencePost);
            }
        }
    }
}

void GameRenderer::renderStickyPuddles(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                      const std::vector<StickyPuddle> &puddles, int groundMathY, int hudTick)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    for (const auto &puddle : puddles)
    {
        const double rad = puddle.currentRadius();
        if (rad < 0.5) continue;

        const int x0 = static_cast<int>(std::round(puddle.mathX - rad));
        const int x1 = static_cast<int>(std::round(puddle.mathX + rad));
        // Mud and ice fade out as they melt away; slime keeps its original look
        const int alpha = (puddle.type == PatchType::SLIME)
                              ? 255
                              : static_cast<int>(255.0 * std::clamp(puddle.lifeTicks / static_cast<double>(StickyPuddle::MELT_TICKS), 0.0, 1.0));

        for (int px = x0; px <= x1; ++px)
        {
            const double normDist = std::abs(px - puddle.mathX) / std::max(1.0, rad);
            if (normDist > 1.0) continue;

            const int sx = grid.mathToScreenX(px);
            const int syTop = grid.mathToScreenY(groundMathY + 1);
            const int syMid = grid.mathToScreenY(groundMathY);

            if (puddle.type == PatchType::SLIME)
            {
                const bool bubble = ((hudTick / 4 + px * 3) % 5 == 0);
                const QColor slimeBase(85, 107, 47);
                const QColor slimeDark(55, 75, 30);
                const QColor slimeGlint(139, 195, 74);
                painter.fillRect(sx, syTop, scale, scale, bubble ? slimeGlint : slimeBase);
                painter.fillRect(sx, syMid, scale, scale, slimeDark);
            }
            else if (puddle.type == PatchType::MUD)
            {
                // Rain-soaked mud: dark, glossy and bubbling. While forming it is only a damp stain.
                const bool bubble = ((hudTick / 5 + px * 5) % 7 == 0);
                QColor top = bubble ? QColor(158, 116, 82) : (((px + puddle.ageTicks / 12) % 3 == 0) ? QColor(104, 74, 52) : QColor(86, 60, 42));
                QColor mid(52, 36, 26);
                if (normDist > 0.8) top = QColor(122, 92, 68);
                const int a = puddle.isForming() ? std::min(alpha, 140) : alpha;
                top.setAlpha(a);
                mid.setAlpha(a);
                painter.fillRect(sx, syTop, scale, scale, top);
                painter.fillRect(sx, syMid, scale, scale, mid);
            }
            else // ICE
            {
                // Glassy frost: bright glints sweep across it; sparkles rise while it is forming
                const bool glint = ((hudTick / 3 + px * 2) % 11 == 0);
                QColor top = glint ? QColor(255, 255, 255) : (normDist > 0.75 ? QColor(150, 215, 240) : QColor(190, 238, 252));
                QColor mid(110, 190, 224);
                const int a = puddle.isForming() ? std::min(alpha, 170) : alpha;
                top.setAlpha(a);
                mid.setAlpha(a);
                painter.fillRect(sx, syTop, scale, scale, top);
                painter.fillRect(sx, syMid, scale, scale, mid);
                if (normDist < 0.5)
                {
                    QColor dome(224, 248, 255);
                    dome.setAlpha(a);
                    painter.fillRect(sx, grid.mathToScreenY(groundMathY + 2), scale, scale, dome);
                }
                if (puddle.isForming() && (hudTick + px) % 4 == 0)
                {
                    painter.fillRect(sx, grid.mathToScreenY(groundMathY + 3 + (hudTick / 2 + px) % 3), scale, scale, QColor(255, 255, 255, 220));
                }
            }
        }
    }
}

// -------------------------------------------------------------
// Gameplay weather overlays
// -------------------------------------------------------------
void GameRenderer::renderWeatherOverlays(QPainter &painter, const GameEngine &engine, const MyGrid &grid,
                                         int scale, int width, int height, int minMathX, int maxMathX, int hudTick)
{
    const double wind = engine.getWind();
    const double wet = engine.getWetness();
    const int groundY = engine.getGroundMathY();
    const int maxMathY = grid.screenToMathY(0);

    // --- Rain shower (thunderstorms draw their own heavier rain) ---
    if (engine.isRainShowerActive() && !engine.isThunderstormActive())
    {
        const int slantPx = static_cast<int>(std::round((wind * 0.8 - 0.30) * scale));
        const int totalFallH = std::max(20, maxMathY - groundY);
        for (int rx = minMathX; rx <= maxMathX; rx += 6)
        {
            const int offset = (hudTick * 3 + ((rx * 29) % 83)) % totalFallH;
            const int ry = maxMathY - offset;
            const int sx = grid.mathToScreenX(rx);
            const int sy = grid.mathToScreenY(ry);
            for (int k = 0; k < 3; ++k)
            {
                painter.fillRect(sx + k * slantPx, sy + k * scale, std::max(2, scale * 2 / 3), scale, QColor(190, 218, 245, 165));
            }
        }
    }

    // --- Gloomy tint + puddle shimmer while the ground is wet ---
    if (wet > 0.02)
    {
        painter.fillRect(0, 0, width, height, QColor(22, 34, 62, static_cast<int>(46 * wet)));
        const int gy = grid.mathToScreenY(groundY + 1);
        for (int px = minMathX; px <= maxMathX; px += 3)
        {
            if ((px * 13 + hudTick / 5) % 9 == 0)
            {
                painter.fillRect(grid.mathToScreenX(px), gy, scale * 2, std::max(1, scale / 2), QColor(214, 236, 255, static_cast<int>(110 * wet)));
            }
        }
    }

    // --- Wind: streaks race across the sky, direction and density follow the gale ---
    const double aw = std::abs(wind);
    if (aw > 0.06)
    {
        const int dir = wind > 0 ? 1 : -1;
        const int streaks = 6 + static_cast<int>(aw * 18);
        const int skyTop = grid.mathToScreenY(maxMathY - 6);
        const int skyBot = grid.mathToScreenY(groundY + 14);
        for (int i = 0; i < streaks; ++i)
        {
            const int h = (i * 7919 + 13) % 1000;
            const int y = skyTop + (skyBot - skyTop) * ((i * 211 + h) % 100) / 100;
            const int len = 36 + (h % 60) + static_cast<int>(aw * 40);
            const int speed = 7 + static_cast<int>(aw * 16) + (h % 5);
            const int cycle = width + 260;
            int x = (hudTick * speed * dir + i * 197) % cycle;
            if (x < 0) x += cycle;
            x -= 130;
            const int a = static_cast<int>(70 + 110 * aw);
            painter.fillRect(dir > 0 ? x - len : x, y, len, std::max(2, scale * 2 / 3), QColor(235, 246, 255, a));
        }
    }

    // --- Gust warning: chevrons flash on the edge the wind will arrive from ---
    if (engine.getWindPhase() == WindPhase::WARNING)
    {
        const double dir = engine.getWindIncomingDirection();
        const bool on = (hudTick / 4) % 2 == 0;
        if (on)
        {
            QFont f("Segoe UI", 30, QFont::Black);
            painter.setFont(f);
            const QString arrows = dir > 0 ? "▶▶▶" : "◀◀◀";
            const QRect r(dir > 0 ? 12 : width - 160, height / 3, 148, 50);
            painter.setPen(QColor(0, 0, 0, 120));
            painter.drawText(r.translated(2, 2), Qt::AlignCenter, arrows);
            painter.setPen(QColor(200, 232, 255, 230));
            painter.drawText(r, Qt::AlignCenter, arrows);
        }
    }
}

// -------------------------------------------------------------
// Night fog
// -------------------------------------------------------------
void GameRenderer::renderFog(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                             int groundMathY, double density, int hudTick)
{
    Q_UNUSED(height);
    if (density < 0.02) return;

    // Fog reaches down to a "fog line" above the ground; below it the air is clear
    const int fogBottom = grid.mathToScreenY(groundMathY + 40);
    if (fogBottom <= 0) return;

    const int aTop = static_cast<int>(std::min(235.0, density * 255.0 * 0.92));
    QLinearGradient grad(0, 0, 0, fogBottom);
    grad.setColorAt(0.0, QColor(196, 208, 226, aTop));
    grad.setColorAt(0.55, QColor(196, 208, 226, static_cast<int>(aTop * 0.78)));
    grad.setColorAt(1.0, QColor(196, 208, 226, 0));
    painter.fillRect(0, 0, width, fogBottom, grad);

    // Drifting chunky wisps so the fog feels alive
    const int bandH = scale * 3;
    for (int k = 0; k < 7; ++k)
    {
        const int dir = (k % 2 == 0) ? 1 : -1;
        const int cycle = width + 500;
        int x = static_cast<int>(hudTick * (0.35 + 0.08 * k)) * dir + k * 233;
        x %= cycle;
        if (x < 0) x += cycle;
        x -= 250;
        const int y = fogBottom * (k + 1) / 9;
        const int w = 200 + k * 34;
        const int a = static_cast<int>(70 * density * (1.0 - static_cast<double>(k) / 10.0));
        painter.fillRect(x, y, w, bandH, QColor(232, 238, 248, a));
        painter.fillRect(x + w / 5, y - bandH / 2, w * 3 / 5, bandH / 2, QColor(232, 238, 248, a * 3 / 4));
    }
}

// -------------------------------------------------------------
// Boss HUD: health bar + warning / victory banners
// -------------------------------------------------------------
void GameRenderer::renderBossHud(QPainter &painter, const GameEngine &engine, int width, int height, int hudTick)
{
    const BossPhase phase = engine.getBossPhase();
    const BossBird &boss = engine.getBoss();
    if (!boss.present || !boss.def) return;

    const BossPalette &pal = boss.def->pal;

    // ---------- Health bar ----------
    const int barW = std::min(static_cast<int>(width * 0.46), 420);
    const int barH = 14;
    const int barX = (width - barW) / 2;
    const int barY = 30;

    double ratio = static_cast<double>(boss.hp) / std::max(1, boss.maxHp);
    if (phase == BossPhase::INTRO)
    {
        ratio = std::min(1.0, engine.getBossPhaseTicks() / 70.0);   // bar fills while the boss arrives
    }

    QFont nameFont("Segoe UI", 10, QFont::Black);
    painter.setFont(nameFont);
    const QString nameStr = QString("%1%2").arg(QString::fromUtf8(boss.def->name), boss.enraged ? "  💢 ENRAGED" : "");
    const QRect nameRect(barX, barY - 20, barW, 18);
    painter.setPen(QColor(0, 0, 0, 200));
    painter.drawText(nameRect.translated(1, 1), Qt::AlignCenter, nameStr);
    painter.setPen(boss.enraged ? QColor(255, 110, 110) : QColor(255, 236, 179));
    painter.drawText(nameRect, Qt::AlignCenter, nameStr);

    painter.setPen(QPen(QColor(10, 12, 20), 2));
    painter.setBrush(QColor(28, 20, 28, 230));
    painter.drawRoundedRect(barX - 2, barY - 2, barW + 4, barH + 4, 5, 5);

    const int fillW = static_cast<int>(barW * ratio);
    QColor fillA = boss.enraged ? QColor(255, 82, 82) : (ratio > 0.6 ? QColor(46, 204, 113) : QColor(255, 183, 3));
    QColor fillB = boss.enraged ? QColor(183, 28, 28) : (ratio > 0.6 ? QColor(20, 140, 80) : QColor(214, 120, 0));
    if (boss.hurtFlash > 0 && boss.hurtFlash % 2 == 1)
    {
        fillA = QColor(255, 255, 255);
        fillB = QColor(255, 236, 179);
    }
    if (fillW > 0)
    {
        QLinearGradient g(0, barY, 0, barY + barH);
        g.setColorAt(0.0, fillA);
        g.setColorAt(1.0, fillB);
        painter.setPen(Qt::NoPen);
        painter.setBrush(g);
        painter.drawRoundedRect(barX, barY, fillW, barH, 3, 3);
    }

    // HP ticks and the rage threshold notch (40%)
    painter.setPen(QColor(0, 0, 0, 90));
    if (boss.maxHp <= 30)
    {
        for (int i = 1; i < boss.maxHp; ++i)
        {
            const int tx = barX + barW * i / boss.maxHp;
            painter.drawLine(tx, barY + 1, tx, barY + barH - 1);
        }
    }
    const int rageX = barX + static_cast<int>(barW * 0.40);
    painter.setPen(QPen(boss.enraged ? QColor(255, 255, 255, 220) : QColor(255, 120, 120, 230), 2));
    painter.drawLine(rageX, barY - 4, rageX, barY + barH + 3);

    QFont hpFont("Segoe UI", 8, QFont::Bold);
    painter.setFont(hpFont);
    const QString hpStr = QString("%1 / %2").arg(std::max(0, boss.hp)).arg(boss.maxHp);
    painter.setPen(QColor(0, 0, 0, 200));
    painter.drawText(QRect(barX + 1, barY + 1, barW, barH), Qt::AlignCenter, hpStr);
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(QRect(barX, barY, barW, barH), Qt::AlignCenter, hpStr);

    // ---------- Banners ----------
    const int ticks = engine.getBossBannerTicks();
    if (phase == BossPhase::INTRO && ticks > 0)
    {
        const int total = engine.getBossBannerMax();
        const int fadeIn = std::min(1.0, (total - ticks) / 10.0) * 255;
        const int fadeOut = std::min(1.0, ticks / 15.0) * 255;
        const int alpha = std::min(fadeIn, fadeOut);

        // pulsing red edge vignette
        const int pulse = static_cast<int>(70 + 60 * std::sin(hudTick * 0.35));
        QRadialGradient vg(width / 2.0, height / 2.0, std::max(width, height) * 0.7);
        vg.setColorAt(0.55, QColor(200, 0, 0, 0));
        vg.setColorAt(1.0, QColor(200, 0, 0, pulse * alpha / 255));
        painter.fillRect(0, 0, width, height, vg);

        const bool blink = (hudTick / 5) % 2 == 0;
        QFont warnFont("Segoe UI", 30, QFont::Black);
        painter.setFont(warnFont);
        const QRect warnRect(0, height / 3 - 56, width, 56);
        painter.setPen(QColor(0, 0, 0, alpha * 2 / 3));
        painter.drawText(warnRect.translated(3, 3), Qt::AlignCenter, "⚠ WARNING ⚠");
        painter.setPen(blink ? QColor(255, 82, 82, alpha) : QColor(255, 214, 10, alpha));
        painter.drawText(warnRect, Qt::AlignCenter, "⚠ WARNING ⚠");

        QFont bossFont("Segoe UI", 24, QFont::Black);
        painter.setFont(bossFont);
        const QRect bossRect(0, height / 3, width, 44);
        painter.setPen(QColor(0, 0, 0, alpha * 2 / 3));
        painter.drawText(bossRect.translated(3, 3), Qt::AlignCenter, QString::fromUtf8(boss.def->name));
        QColor nc = pal.crestLight;
        nc.setAlpha(alpha);
        painter.setPen(nc);
        painter.drawText(bossRect, Qt::AlignCenter, QString::fromUtf8(boss.def->name));

        QFont titleFont("Segoe UI", 13, QFont::DemiBold);
        painter.setFont(titleFont);
        painter.setPen(QColor(224, 230, 237, alpha));
        painter.drawText(QRect(0, height / 3 + 46, width, 26), Qt::AlignCenter, QString::fromUtf8(boss.def->title));
    }
    else if (phase == BossPhase::DEFEATED && ticks > 0)
    {
        const int alpha = std::min(255, ticks * 8);
        QFont f("Segoe UI", 32, QFont::Black);
        painter.setFont(f);
        const QRect r(0, height / 3 - 30, width, 60);
        painter.setPen(QColor(0, 0, 0, alpha * 2 / 3));
        painter.drawText(r.translated(3, 3), Qt::AlignCenter, "🏆 BOSS DEFEATED! 🏆");
        painter.setPen(QColor(255, 214, 10, alpha));
        painter.drawText(r, Qt::AlignCenter, "🏆 BOSS DEFEATED! 🏆");

        QFont sf("Segoe UI", 13, QFont::DemiBold);
        painter.setFont(sf);
        painter.setPen(QColor(224, 230, 237, alpha));
        painter.drawText(QRect(0, height / 3 + 32, width, 26), Qt::AlignCenter, "Catch the 👑 Trophy Egg!");
    }
}

QPixmap GameRenderer::renderFrame(const GameEngine &engine,
                                  MyGrid &grid,
                                  MyPixels &pixels,
                                  int width,
                                  int height,
                                  int scale,
                                  int hudTick,
                                  const AudioManager *audio)
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

    const double dnProg = engine.getDayNightProgress();
    const double nightRatio = engine.getNightDurationRatio();
    const bool isStorm = engine.isThunderstormActive();
    const int flashTicks = engine.isLightningFlashEnabled() ? engine.getLightningFlashTicks() : 0;

    // -------------------------------------------------------------
    // BACKGROUND PASS: 100% Pure Pixelated World
    // -------------------------------------------------------------
    // 1. Pixelated Sky
    renderPixelSky(painter, grid, scale, width, height, minMathX, maxMathX, minMathY, maxMathY,
                   engine.getGroundMathY(), dnProg, nightRatio, isStorm, flashTicks, hudTick);

    // 2. Mountains & Hills (with ZERO gaps, fills completely to groundMathY)
    renderPixelHills(painter, grid, scale, width, height, minMathX, maxMathX, engine.getGroundMathY(),
                     dnProg, nightRatio, isStorm);

    // 3. Farmhouse & barn elevated on midground knoll
    renderPixelFarmhouse(painter, grid, scale, width, height, engine.getGroundMathY(), hudTick,
                         dnProg, nightRatio, isStorm);

    // 4. Rustic wooden fence
    renderPixelFence(painter, grid, scale, width, height, minMathX, maxMathX, engine.getGroundMathY(),
                     dnProg, nightRatio, isStorm);

    // 5. Sticky Poop Puddles on the ground
    renderStickyPuddles(painter, grid, scale, width, height, engine.getStickyPuddles(), engine.getGroundMathY(), hudTick);

    // -------------------------------------------------------------
    // MIDGROUND: BIRDS PASS (Rendered behind drifting clouds)
    // -------------------------------------------------------------
    MyPixels birdPixels;

    for (const auto &bird : engine.getBirds())
    {
        rasterizeBird(birdPixels, bird);
    }

    if (engine.isLegendaryBirdActive())
    {
        rasterizeBird(birdPixels, engine.getLegendaryBird());
    }

    if (engine.getBoss().present)
    {
        rasterizeBossBird(birdPixels, engine.getBoss());
    }

    for (const auto &entry : birdPixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        paintCell(painter, grid, scale, width, height, x, y, entry.second);
    }

    // -------------------------------------------------------------
    // BIRD ATTACK TELEGRAPH BEAM & GROUND TARGET RETICLE
    // -------------------------------------------------------------
    for (const auto &bird : engine.getBirds())
    {
        if (bird.attackState == Bird::AttackState::TELEGRAPH)
        {
            const int bx = grid.mathToScreenX(static_cast<int>(std::round(bird.mathX)));
            const int by = grid.mathToScreenY(static_cast<int>(std::round(bird.mathY)));
            const int tx = grid.mathToScreenX(static_cast<int>(std::round(bird.attackTargetX)));
            const int ty = grid.mathToScreenY(engine.getGroundMathY() + 1);

            const int pulseAlpha = (bird.attackTicks % 8 < 4) ? 140 : 60;
            QPen laserPen(QColor(255, 30, 30, pulseAlpha), std::max(1, scale / 2), Qt::DashLine);
            painter.setPen(laserPen);
            painter.drawLine(bx + scale / 2, by + scale / 2, tx + scale / 2, ty + scale / 2);

            // Ground warning target box
            const int rSize = scale * 10;
            painter.setPen(QPen(QColor(255, 50, 50, pulseAlpha + 50), 2));
            painter.setBrush(QColor(255, 30, 30, pulseAlpha / 3));
            painter.drawRect(tx - rSize / 2, ty - scale * 2, rSize, scale * 3);
        }
    }

    // -------------------------------------------------------------
    // DRIFTING PIXEL CLOUDS (Rendered in front of birds so birds hide behind clouds)
    // -------------------------------------------------------------
    renderPixelClouds(painter, grid, scale, width, height, minMathX, maxMathX, maxMathY,
                      engine.getDayNightTick(), dnProg, nightRatio, isStorm);

    // -------------------------------------------------------------
    // FOREGROUND RASTERIZATION PASS
    // -------------------------------------------------------------
    pixels.clear();

    // Ground
    rasterizeGround(pixels, minMathX, maxMathX, engine.getGroundMathY(), minMathY);

    // Player Basket
    rasterizeBasket(pixels, engine.getBasket());

    // Companion Basket (Hard Mode)
    if (engine.getCompanionBasket().active)
    {
        rasterizeCompanionBasket(pixels, engine.getCompanionBasket());
    }

    // Falling Eggs
    for (const auto &egg : engine.getFallingEggs())
    {
        if (egg.active)
        {
            rasterizeEgg(pixels, egg);
        }
    }

    // Hearts & Level Progress Bar & Pure Raster Pixel Egg Status
    if (engine.getState() != GameState::MENU)
    {
        const int barRight = maxMathX - 16;
        const int barLeft = barRight - 36;
        rasterizePixelHearts(pixels, minMathX + 3, maxMathY - 3, engine.getHearts(), 3);
        rasterizeLevelBar(pixels, barLeft, barRight, maxMathY - 2, engine.getLevel(), engine.getProgressScore(), hudTick);
        rasterizeEggStatus(pixels, barLeft, maxMathY - 11, engine.getGrowthEggCount(), engine.getMode() == GameMode::HARD, hudTick);
    }

    // -------------------------------------------------------------
    // PASS 1: Pixel Glow Aura Pass
    // -------------------------------------------------------------
    const int glowRadius = (scale <= 3 ? 1 : (scale <= 6 ? 2 : 3));
    const int glowAlpha = (scale <= 3 ? 32 : 45);

    // Glowing Auras for Eggs
    for (const auto &egg : engine.getFallingEggs())
    {
        if (!egg.active) continue;

        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY)));

        if (egg.type == EggType::GOLDEN)
        {
            const int auraR = scale * 7;
            QRadialGradient goldGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
            goldGlow.setColorAt(0.0, QColor(255, 215, 0, 80));
            goldGlow.setColorAt(0.4, QColor(255, 190, 0, 30));
            goldGlow.setColorAt(1.0, QColor(255, 180, 0, 0));
            painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, goldGlow);
        }
        else if (egg.type == EggType::BASKET_GROW)
        {
            const int auraR = scale * 7;
            QRadialGradient growGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
            growGlow.setColorAt(0.0, QColor(46, 204, 113, 85));
            growGlow.setColorAt(0.4, QColor(39, 174, 96, 30));
            growGlow.setColorAt(1.0, QColor(39, 174, 96, 0));
            painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, growGlow);
        }
        else if (egg.type == EggType::BASKET_RESTORE)
        {
            const int auraR = scale * 8;
            QRadialGradient restoreGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
            restoreGlow.setColorAt(0.0, QColor(0, 229, 255, 95));
            restoreGlow.setColorAt(0.4, QColor(0, 180, 216, 40));
            restoreGlow.setColorAt(1.0, QColor(0, 229, 255, 0));
            painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, restoreGlow);
        }
        else if (egg.type == EggType::BOSS_REWARD)
        {
            const int auraR = scale * 10;
            QRadialGradient trophyGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
            trophyGlow.setColorAt(0.0, QColor(255, 214, 10, 120));
            trophyGlow.setColorAt(0.45, QColor(187, 134, 252, 55));
            trophyGlow.setColorAt(1.0, QColor(142, 68, 173, 0));
            painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, trophyGlow);
        }
        else if (egg.type == EggType::BIRD_POOP)
        {
            const int stinkR = scale * 6;
            QRadialGradient stinkGlow(ex + scale / 2.0, ey + scale / 2.0, stinkR);
            stinkGlow.setColorAt(0.0, QColor(139, 195, 74, 80));
            stinkGlow.setColorAt(0.5, QColor(109, 76, 65, 35));
            stinkGlow.setColorAt(1.0, QColor(139, 195, 74, 0));
            painter.fillRect(ex + scale / 2 - stinkR, ey + scale / 2 - stinkR, stinkR * 2, stinkR * 2, stinkGlow);
        }

        // Telegraphed Boss Egg: Radiant beacon aura & ground landing indicator
        if (egg.telegraphed)
        {
            const int auraR = scale * 9;
            const int sparklePulse = static_cast<int>(35 * std::sin(hudTick * 0.45));
            QRadialGradient bossEggGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
            bossEggGlow.setColorAt(0.0, QColor(255, 235, 59, 130 + sparklePulse));
            bossEggGlow.setColorAt(0.4, QColor(255, 193, 7, 50));
            bossEggGlow.setColorAt(1.0, QColor(255, 193, 7, 0));
            painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, bossEggGlow);

            const int groundSY = grid.mathToScreenY(engine.getGroundMathY() + 8);
            const int markerW = scale * 12;
            const int markerH = scale * 3;
            QRadialGradient groundMarker(ex + scale / 2.0, groundSY, markerW / 2.0);
            groundMarker.setColorAt(0.0, QColor(255, 215, 0, 90 + sparklePulse));
            groundMarker.setColorAt(0.6, QColor(255, 193, 7, 30));
            groundMarker.setColorAt(1.0, QColor(255, 193, 7, 0));
            painter.fillRect(ex + scale / 2 - markerW / 2, groundSY - markerH / 2, markerW, markerH, groundMarker);
        }
    }

    // Silhouette Birds: Faint Glowing Eye Glint Aura in Fog
    for (const auto &bird : engine.getBirds())
    {
        if (bird.isSilhouette && !bird.inCloud)
        {
            const int bx = grid.mathToScreenX(static_cast<int>(std::round(bird.mathX)));
            const int by = grid.mathToScreenY(static_cast<int>(std::round(bird.mathY)));
            const int eyeX = bx + 4 * bird.direction * scale;
            const int eyeY = by + 2 * scale;
            const int auraR = scale * 5;
            QRadialGradient eyeGlow(eyeX + scale / 2.0, eyeY + scale / 2.0, auraR);
            eyeGlow.setColorAt(0.0, QColor(255, 235, 130, 150));
            eyeGlow.setColorAt(0.4, QColor(255, 215, 0, 50));
            eyeGlow.setColorAt(1.0, QColor(255, 215, 0, 0));
            painter.fillRect(eyeX + scale / 2 - auraR, eyeY + scale / 2 - auraR, auraR * 2, auraR * 2, eyeGlow);
        }
    }

    // Phoenix Solar Aura
    if (engine.isLegendaryBirdActive())
    {
        const Bird &lb = engine.getLegendaryBird();
        const int lx = grid.mathToScreenX(static_cast<int>(std::round(lb.mathX)));
        const int ly = grid.mathToScreenY(static_cast<int>(std::round(lb.mathY)));
        const int auraR = scale * 10;
        QRadialGradient phoenixGlow(lx + scale / 2.0, ly + scale / 2.0, auraR);
        phoenixGlow.setColorAt(0.0, QColor(255, 238, 88, 110));
        phoenixGlow.setColorAt(0.35, QColor(255, 112, 67, 55));
        phoenixGlow.setColorAt(0.7, QColor(229, 57, 53, 20));
        phoenixGlow.setColorAt(1.0, QColor(244, 81, 30, 0));
        painter.fillRect(lx + scale / 2 - auraR, ly + scale / 2 - auraR, auraR * 2, auraR * 2, phoenixGlow);
    }

    // Boss aura: tinted by its palette, red and throbbing when enraged, belly flares while charging
    if (engine.getBoss().present && engine.getBoss().def)
    {
        const BossBird &bb = engine.getBoss();
        const int bx = grid.mathToScreenX(static_cast<int>(std::round(bb.mathX)));
        const int by = grid.mathToScreenY(static_cast<int>(std::round(bb.mathY)));
        const int auraR = scale * 36;
        QColor core = bb.def->pal.glow;
        int a0 = 70;
        if (bb.enraged)
        {
            core = lerpColor(core, QColor(255, 60, 60), 0.75);
            a0 = static_cast<int>(95 + 40 * std::sin(hudTick * 0.3));
        }
        QRadialGradient bossGlow(bx + scale / 2.0, by + scale / 2.0, auraR);
        bossGlow.setColorAt(0.0, QColor(core.red(), core.green(), core.blue(), a0));
        bossGlow.setColorAt(0.5, QColor(core.red(), core.green(), core.blue(), a0 / 3));
        bossGlow.setColorAt(1.0, QColor(core.red(), core.green(), core.blue(), 0));
        painter.fillRect(bx + scale / 2 - auraR, by + scale / 2 - auraR, auraR * 2, auraR * 2, bossGlow);

        if (bb.chargeTicks > 0)
        {
            const double charge = 1.0 - static_cast<double>(bb.chargeTicks) / std::max(1, bb.chargeMax);
            const int py = grid.mathToScreenY(static_cast<int>(std::round(bb.mathY - 7.5)));
            const int pr = static_cast<int>(scale * (6 + 10 * charge));
            QRadialGradient pod(bx + scale / 2.0, py + scale / 2.0, pr);
            pod.setColorAt(0.0, QColor(255, 255, 255, static_cast<int>(170 * charge)));
            pod.setColorAt(0.4, QColor(bb.def->pal.glow.red(), bb.def->pal.glow.green(), bb.def->pal.glow.blue(), static_cast<int>(120 * charge)));
            pod.setColorAt(1.0, QColor(bb.def->pal.glow.red(), bb.def->pal.glow.green(), bb.def->pal.glow.blue(), 0));
            painter.fillRect(bx + scale / 2 - pr, py + scale / 2 - pr, pr * 2, pr * 2, pod);
        }
    }

    // Companion Basket Celestial Aura
    if (engine.getCompanionBasket().active)
    {
        const CompanionBasket &comp = engine.getCompanionBasket();
        const int cx = grid.mathToScreenX(static_cast<int>(std::round(comp.mathX)));
        const int cy = grid.mathToScreenY(static_cast<int>(std::round(comp.mathY)));
        const int auraR = scale * 8;
        QRadialGradient compGlow(cx + scale / 2.0, cy + scale / 2.0, auraR);
        compGlow.setColorAt(0.0, QColor(0, 229, 255, 90));
        compGlow.setColorAt(0.4, QColor(255, 215, 0, 40));
        compGlow.setColorAt(1.0, QColor(0, 229, 255, 0));
        painter.fillRect(cx + scale / 2 - auraR, cy + scale / 2 - auraR, auraR * 2, auraR * 2, compGlow);
    }

    // Shocked Basket Electrical Aura
    if (engine.getLightningShockTimer() > 0)
    {
        const Basket &b = engine.getBasket();
        const int bx = grid.mathToScreenX(static_cast<int>(std::round(b.mathX)));
        const int by = grid.mathToScreenY(static_cast<int>(std::round(b.mathY)));
        const int auraR = scale * 9;
        QRadialGradient shockGlow(bx + scale / 2.0, by + scale / 2.0, auraR);
        shockGlow.setColorAt(0.0, QColor(0, 229, 255, 110));
        shockGlow.setColorAt(0.5, QColor(255, 235, 59, 50));
        shockGlow.setColorAt(1.0, QColor(0, 229, 255, 0));
        painter.fillRect(bx + scale / 2 - auraR, by + scale / 2 - auraR, auraR * 2, auraR * 2, shockGlow);
    }

    // Raster Pixels Aura
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

    // Particle Auras
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
    // PASS 2: Pixel Core Pass
    // -------------------------------------------------------------
    for (const auto &entry : pixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        paintCell(painter, grid, scale, width, height, x, y, entry.second);
    }

    for (const auto &p : particles)
    {
        paintCell(painter, grid, scale, width, height,
                  static_cast<int>(std::round(p.x)),
                  static_cast<int>(std::round(p.y)),
                  p.color);
    }

    // -------------------------------------------------------------
    // THUNDERSTORM: Storm Clouds Overlay Covering Sheltered Birds
    // -------------------------------------------------------------
    if (isStorm)
    {
        // Torrential pixel rain streaks across the viewport
        for (int rx = minMathX; rx <= maxMathX; rx += 4)
        {
            const int totalFallH = std::max(20, maxMathY - engine.getGroundMathY());
            const int offset = (hudTick * 4 + ((rx * 17) % 73)) % totalFallH;
            const int ry = maxMathY - offset;
            const int sx = grid.mathToScreenX(rx);
            const int sy = grid.mathToScreenY(ry);
            painter.fillRect(sx, sy, scale, scale * 3, QColor(176, 205, 235, 160));
        }
    }

    // -------------------------------------------------------------
    // THUNDER STRIKE: Ground Warning & Lethal Lightning Bolt
    // -------------------------------------------------------------
    if (engine.getLightningStrikeState() == 1)
    {
        // Warning ionization sizzling on ground
        const int strikeSX = grid.mathToScreenX(static_cast<int>(std::round(engine.getLightningStrikeX())));
        const int groundSY = grid.mathToScreenY(engine.getGroundMathY() + 1);
        const bool warnBlink = (hudTick % 4 < 2);

        // Warning pillar
        painter.fillRect(strikeSX - scale, groundSY - scale * 6, scale * 2, scale * 6,
                         warnBlink ? QColor(255, 235, 59, 180) : QColor(0, 229, 255, 140));

        QFont warnFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(warnFont);
        painter.setPen(warnBlink ? QColor(255, 235, 59) : QColor(0, 229, 255));
        painter.drawText(QRect(strikeSX - 45, groundSY - 35, 90, 20), Qt::AlignCenter, "⚡ DANGER ⚡");
    }
    else if (engine.getLightningStrikeState() == 2)
    {
        // Impact! Massive jagged lightning bolt crashing down!
        const int strikeMathX = static_cast<int>(std::round(engine.getLightningStrikeX()));
        int curX = strikeMathX;

        for (int my = maxMathY - 14; my >= engine.getGroundMathY() + 1; --my)
        {
            const int sx = grid.mathToScreenX(curX);
            const int sy = grid.mathToScreenY(my);

            // Blinding white core with electric cyan aura
            painter.fillRect(sx - scale, sy, scale * 3, scale, QColor(0, 229, 255, 160));
            painter.fillRect(sx, sy, scale, scale, QColor(255, 255, 255));

            if (my % 3 == 0)
            {
                curX += ((my / 3) % 2 == 0 ? 1 : -1);
            }
        }
    }

    // -------------------------------------------------------------
    // GAMEPLAY WEATHER: rain/wind overlays, then night fog (hides eggs up high)
    // -------------------------------------------------------------
    if (engine.getState() == GameState::PLAYING || engine.getState() == GameState::PAUSED)
    {
        renderWeatherOverlays(painter, engine, grid, scale, width, height, minMathX, maxMathX, hudTick);
        renderFog(painter, grid, scale, width, height, engine.getGroundMathY(), engine.getFogDensity(), hudTick);
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

    // Companion Basket Overhead Badge
    if (engine.getCompanionBasket().active)
    {
        const CompanionBasket &comp = engine.getCompanionBasket();
        const int cx = grid.mathToScreenX(static_cast<int>(std::round(comp.mathX)));
        const int cy = grid.mathToScreenY(static_cast<int>(std::round(comp.mathY + comp.height + 4)));
        const double secs = std::max(0.0, comp.ticksLeft / 33.0);
        const QString compStr = QString("🤖 COMPANION (%1s)").arg(QString::number(secs, 'f', 1));

        QFont compFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(compFont);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(QRect(cx - 60, cy - 10, 120, 20), Qt::AlignCenter, compStr);
        painter.setPen(QColor(0, 229, 255));
        painter.drawText(QRect(cx - 61, cy - 11, 120, 20), Qt::AlignCenter, compStr);
    }

    // Shocked Basket Overhead Badge
    if (engine.getLightningShockTimer() > 0)
    {
        const Basket &b = engine.getBasket();
        const int bx = grid.mathToScreenX(static_cast<int>(std::round(b.mathX)));
        const int by = grid.mathToScreenY(static_cast<int>(std::round(b.mathY + b.height + 4)));
        const double secs = std::max(0.0, engine.getLightningShockTimer() / 33.0);
        const QString shockStr = QString("⚡ SHOCKED (%1s)").arg(QString::number(secs, 'f', 1));

        QFont shockFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(shockFont);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(QRect(bx - 60, by - 10, 120, 20), Qt::AlignCenter, shockStr);
        painter.setPen(QColor(255, 235, 59));
        painter.drawText(QRect(bx - 61, by - 11, 120, 20), Qt::AlignCenter, shockStr);
    }

    // Surface badges: what is under (or happening to) the basket
    if (engine.getState() == GameState::PLAYING)
    {
        const Basket &b = engine.getBasket();
        const int bx = grid.mathToScreenX(static_cast<int>(std::round(b.mathX)));
        const int by = grid.mathToScreenY(static_cast<int>(std::round(b.mathY + b.height + 4)));
        int row = (engine.getLightningShockTimer() > 0) ? 1 : 0;
        auto drawBadge = [&](const QString &str, const QColor &col) {
            QFont badgeFont("Segoe UI", 9, QFont::Bold);
            painter.setFont(badgeFont);
            const int yOff = by - 11 - row * 16;
            painter.setPen(QColor(0, 0, 0, 200));
            painter.drawText(QRect(bx - 70, yOff, 140, 20), Qt::AlignCenter, str);
            painter.setPen(col);
            painter.drawText(QRect(bx - 71, yOff - 1, 140, 20), Qt::AlignCenter, str);
            ++row;
        };
        if (engine.isBasketOnPatch())
        {
            switch (engine.getBasketPatchType())
            {
            case PatchType::ICE:   drawBadge("🧊 ICE: SLIDING!", QColor(150, 235, 255)); break;
            case PatchType::MUD:   drawBadge("🟤 MUD: HEAVY SLOW", QColor(205, 160, 112)); break;
            case PatchType::SLIME: drawBadge("SLIME: SLOW", QColor(139, 195, 74)); break;
            }
        }
        if (engine.getWetness() > 0.4)
        {
            drawBadge("🌧 SLIPPERY", QColor(150, 195, 240));
        }
    }

    // HUD Text: Level, Score, High Score & Mode
    const int level = engine.getLevel();
    const int score = engine.getScore();
    if (engine.getState() == GameState::PLAYING || engine.getState() == GameState::PAUSED)
    {
        const int heartsStartX = grid.mathToScreenX(minMathX + 3);
        const int heartsBottomY = grid.mathToScreenY(maxMathY - 7);
        const int hudLeftX = heartsStartX;
        const int hudStartY = heartsBottomY + 28;

        // Level Number
        QFont hudLevelFont("Segoe UI", 11, QFont::Bold);
        painter.setFont(hudLevelFont);
        QString levelStr = QString("LEVEL %1").arg(level);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 1, levelStr);
        painter.setPen(QColor(199, 125, 255));
        painter.drawText(hudLeftX, hudStartY, levelStr);

        // Current Score
        QFont hudScoreFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(hudScoreFont);
        QString scoreStr = QString("SCORE: %1").arg(score);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 18, scoreStr);
        painter.setPen(QColor(6, 214, 160));
        painter.drawText(hudLeftX, hudStartY + 17, scoreStr);

        // Best Score
        QString bestStr = QString("BEST: %1").arg(engine.getHighScore());
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 35, bestStr);
        painter.setPen(QColor(255, 183, 3));
        painter.drawText(hudLeftX, hudStartY + 34, bestStr);

        // Mode Badge
        QFont modeFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(modeFont);
        QString modeStr;
        QColor modeCol;
        if (engine.getMode() == GameMode::EASY)
        {
            modeStr = "MODE: EASY";
            modeCol = QColor(46, 204, 113);
        }
        else if (engine.getMode() == GameMode::MEDIUM)
        {
            modeStr = "MODE: MEDIUM";
            modeCol = QColor(255, 183, 3);
        }
        else
        {
            modeStr = "MODE: HARD";
            modeCol = QColor(239, 71, 111);
        }
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(hudLeftX + 1, hudStartY + 51, modeStr);
        painter.setPen(modeCol);
        painter.drawText(hudLeftX, hudStartY + 50, modeStr);

        // Weather conditions that change how the game plays
        int wy = hudStartY + 68;
        auto weatherLine = [&](const QString &txt, const QColor &col) {
            painter.setPen(QColor(0, 0, 0, 200));
            painter.drawText(hudLeftX + 1, wy + 1, txt);
            painter.setPen(col);
            painter.drawText(hudLeftX, wy, txt);
            wy += 16;
        };
        const double wnd = engine.getWind();
        if (engine.getWindPhase() == WindPhase::WARNING)
        {
            const bool on = (hudTick / 4) % 2 == 0;
            weatherLine(engine.getWindIncomingDirection() > 0 ? "WIND ▶▶▶ INCOMING" : "◀◀◀ WIND INCOMING",
                        on ? QColor(255, 235, 59) : QColor(190, 225, 255));
        }
        else if (std::abs(wnd) > 0.08)
        {
            const int n = std::abs(wnd) > 0.7 ? 3 : (std::abs(wnd) > 0.35 ? 2 : 1);
            QString arrows;
            for (int i = 0; i < n; ++i) arrows += (wnd > 0 ? "▶" : "◀");
            weatherLine(QString("WIND %1 %2").arg(arrows, n == 3 ? "STRONG" : (n == 2 ? "MEDIUM" : "LIGHT")), QColor(190, 225, 255));
        }
        if (engine.isThunderstormActive())
        {
            weatherLine("STORM: SLIPPERY", QColor(255, 235, 59));
        }
        else if (engine.isRainShowerActive() || engine.getWetness() > 0.4)
        {
            weatherLine("RAIN: SLIPPERY", QColor(150, 195, 240));
        }
        if (engine.getFogDensity() > 0.15)
        {
            weatherLine("FOG: LOW VISIBILITY", QColor(205, 214, 228));
        }

        // Progress Bar Text (under the level bar)
        const bool maxed = level >= GameEngine::MAX_LEVEL;
        const int levelStart = GameEngine::levelStartScore(level);
        const int levelLen = GameEngine::levelLength(level);
        const int earned = maxed ? levelLen : std::clamp(engine.getProgressScore() - levelStart, 0, levelLen);
        const int remaining = levelLen - earned;

        QString progressLabel = maxed ? QString("MAX LEVEL (50)")
                                      : QString("%1 PTS TO NEXT LEVEL").arg(remaining);
        if (engine.getBossPhase() == BossPhase::PENDING)
        {
            progressLabel = "⚠ BOSS INCOMING...";
        }
        else if (engine.isBossActive())
        {
            progressLabel = "⚔ BOSS FIGHT";
        }

        QFont progressFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(progressFont);

        const int barRight = maxMathX - 16;
        const int barLeft = barRight - 36;
        const int sx = grid.mathToScreenX(barLeft);
        const int sy = grid.mathToScreenY(maxMathY - 7);
        const int barW = grid.mathToScreenX(barRight) - sx;

        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(QRect(sx + 1, sy + 1, barW, 20), Qt::AlignCenter, progressLabel);
        painter.setPen(maxed ? QColor(255, 215, 0) : QColor(224, 230, 237));
        painter.drawText(QRect(sx, sy, barW, 20), Qt::AlignCenter, progressLabel);
    }

    // Level-up banner
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

    // Boss health bar + warning/victory banners
    if (engine.getState() == GameState::PLAYING || engine.getState() == GameState::PAUSED)
    {
        renderBossHud(painter, engine, width, height, hudTick);
    }

    // Overlays for Start Menu, Game Over, and Paused
    if (engine.getState() == GameState::MENU)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 13, 20, 228));

        const int cardW = std::min(width - 40, 520);
        const int cardH = std::min(height - 40, 360);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        painter.setPen(QPen(QColor(48, 58, 80), 2));
        painter.setBrush(QColor(20, 24, 36, 248));
        painter.drawRoundedRect(cardX, cardY, cardW, cardH, 12, 12);

        // Header Accent line
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 209, 102));
        painter.drawRoundedRect(cardX + 24, cardY + 50, cardW - 48, 2, 1, 1);

        QFont titleFont("Segoe UI", 20, QFont::Black);
        painter.setFont(titleFont);
        painter.setPen(QColor(0, 0, 0, 200));
        painter.drawText(QRect(cardX + 2, cardY + 10, cardW, 32), Qt::AlignCenter, "EGGSCELLENT CATCH");
        painter.setPen(QColor(255, 209, 102));
        painter.drawText(QRect(cardX, cardY + 8, cardW, 32), Qt::AlignCenter, "EGGSCELLENT CATCH");

        // Settings Button on Main Menu Card Header
        const QRect menuSettingsBtn(cardX + cardW - 95, cardY + 12, 80, 26);
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.setPen(QPen(QColor(0, 229, 255), 1));
        painter.setBrush(QColor(18, 44, 62));
        painter.drawRoundedRect(menuSettingsBtn, 6, 6);
        painter.setPen(QColor(0, 229, 255));
        painter.drawText(menuSettingsBtn, Qt::AlignCenter, "⚙ Settings");

        // Exit Game Button on Main Menu Card Header
        const QRect menuExitBtn(cardX + 15, cardY + 12, 65, 26);
        painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
        painter.setPen(QPen(QColor(239, 71, 111), 1));
        painter.setBrush(QColor(48, 20, 28));
        painter.drawRoundedRect(menuExitBtn, 6, 6);
        painter.setPen(QColor(239, 71, 111));
        painter.drawText(menuExitBtn, Qt::AlignCenter, "✕ Exit");

        // Scrollable Viewport
        const int contentTop = cardY + 54;
        const int viewportH = cardH - 62;
        const int totalContentH = 370;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        const int scrollY = std::clamp(engine.getMenuScrollY(), 0, maxScroll);

        painter.save();
        painter.setClipRect(cardX + 4, contentTop, cardW - 8, viewportH);
        painter.translate(0, -scrollY);

        // Mode pills
        const int modeBtnW = 100;
        const int modeBtnH = 26;
        const int modeBarY = contentTop + 8;
        const int startModeX = cardX + (cardW - (modeBtnW * 3 + 20)) / 2;

        QFont modeBtnFont("Segoe UI", 10, QFont::Bold);
        painter.setFont(modeBtnFont);

        const bool isEasy = (engine.getMode() == GameMode::EASY);
        painter.setPen(QPen(isEasy ? QColor(46, 204, 113) : QColor(60, 75, 90), isEasy ? 2 : 1));
        painter.setBrush(isEasy ? QColor(25, 55, 38) : QColor(26, 32, 44));
        painter.drawRoundedRect(startModeX, modeBarY, modeBtnW, modeBtnH, 6, 6);
        painter.setPen(isEasy ? QColor(46, 204, 113) : QColor(160, 175, 190));
        painter.drawText(QRect(startModeX, modeBarY, modeBtnW, modeBtnH), Qt::AlignCenter, "[1] EASY");

        const bool isMed = (engine.getMode() == GameMode::MEDIUM);
        painter.setPen(QPen(isMed ? QColor(255, 183, 3) : QColor(60, 75, 90), isMed ? 2 : 1));
        painter.setBrush(isMed ? QColor(55, 45, 20) : QColor(26, 32, 44));
        painter.drawRoundedRect(startModeX + modeBtnW + 10, modeBarY, modeBtnW, modeBtnH, 6, 6);
        painter.setPen(isMed ? QColor(255, 183, 3) : QColor(160, 175, 190));
        painter.drawText(QRect(startModeX + modeBtnW + 10, modeBarY, modeBtnW, modeBtnH), Qt::AlignCenter, "[2] MED");

        const bool isHard = (engine.getMode() == GameMode::HARD);
        painter.setPen(QPen(isHard ? QColor(239, 71, 111) : QColor(60, 75, 90), isHard ? 2 : 1));
        painter.setBrush(isHard ? QColor(60, 25, 35) : QColor(26, 32, 44));
        painter.drawRoundedRect(startModeX + (modeBtnW + 10) * 2, modeBarY, modeBtnW, modeBtnH, 6, 6);
        painter.setPen(isHard ? QColor(239, 71, 111) : QColor(160, 175, 190));
        painter.drawText(QRect(startModeX + (modeBtnW + 10) * 2, modeBarY, modeBtnW, modeBtnH), Qt::AlignCenter, "[3] HARD");

        QFont modeSumFont("Segoe UI", 9, QFont::Normal);
        painter.setFont(modeSumFont);
        QString modeSummary;
        if (isEasy)
        {
            modeSummary = "✓ 1 starting bird  •  Calm skies  •  Standard day/night pace";
        }
        else if (isMed)
        {
            modeSummary = "⚡ 2 starting birds  •  Faster scaling  •  Sticky poop puddles  •  Longer night";
        }
        else
        {
            modeSummary = "🔥 2 starting birds  •  Rare dual drops!  •  Large sticky zones  •  [C] Companion";
        }
        painter.setPen(QColor(168, 199, 250));
        painter.drawText(QRect(cardX, modeBarY + 30, cardW, 18), Qt::AlignCenter, modeSummary);

        // Dedicated Start Game Button
        const int startBtnW = std::min(cardW - 70, 320);
        const int startBtnH = 34;
        const int startBtnX = cardX + (cardW - startBtnW) / 2;
        const int startBtnY = modeBarY + 54;
        const QRect startBtnRect(startBtnX, startBtnY, startBtnW, startBtnH);

        painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
        painter.setPen(QPen(QColor(46, 204, 113), 2));
        painter.setBrush(QColor(22, 54, 38));
        painter.drawRoundedRect(startBtnRect, 8, 8);
        painter.setPen(QColor(46, 204, 113));
        painter.drawText(startBtnRect, Qt::AlignCenter, "▶   START GAME   [SPACE]");

        // Slide Content
        const int colX = cardX + 30;
        int y = startBtnY + 44;

        QFont secFont("Segoe UI", 11, QFont::Bold);
        QFont bodyFont("Segoe UI", 9, QFont::Normal);
        QFont boldBodyFont("Segoe UI", 9, QFont::Bold);

        int slideIndex = engine.getMenuSlideIndex();

        if (slideIndex == 0)
        {
            painter.setFont(secFont);
            painter.setPen(QColor(6, 214, 160));
            painter.drawText(colX, y, "HOW TO PLAY");
            y += 20;

            painter.setFont(bodyFont);
            painter.setPen(QColor(224, 230, 237));
            painter.drawText(colX, y, "• Catch falling eggs before they hit the ground!");
            y += 18;
            painter.drawText(colX, y, "• 3 missed eggs = Game Over (lose hearts).");
            y += 18;
            painter.drawText(colX, y, "• Avoid bird poop! Catching poop loses 1 heart (-1 ❤).");
            y += 18;
            painter.drawText(colX, y, "• Birds attack from Level 1! DEFEND with [SPACE] / [🛡️] or DODGE!");
            y += 18;
            painter.drawText(colX, y, "• Catch Green Growth Eggs to store in inventory (Use with [H])!");
            y += 18;
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(199, 125, 255));
            painter.drawText(colX + 10, y, "Level up fast (50 / 70 pts early, then 100): Faster drops & smaller basket!");
            y += 24;
        }
        else if (slideIndex == 1)
        {
            painter.setFont(secFont);
            painter.setPen(QColor(199, 125, 255));
            painter.drawText(colX, y, "CONTROLS");
            y += 20;

            painter.setFont(bodyFont);
            painter.setPen(QColor(224, 230, 237));
            painter.drawText(colX, y, "• Move Basket : Touch / Drag / Mouse / [A / D] / [Left / Right]");
            y += 18;
            painter.drawText(colX, y, "• Defend Basket : Press [SPACE] / Right-Click / 🛡️ DEFEND button");
            y += 18;
            painter.drawText(colX, y, "• Use Growth Egg : Press [H] or 🧺 Button (Expands basket)");
            y += 18;
            painter.drawText(colX, y, "• Summon Companion : Press [C] or 🤖 Button (Hard mode: 2 eggs)");
            y += 18;
            painter.drawText(colX, y, "• Switch Difficulty : Press [1], [2], [3] or [TAB]");
            y += 18;
            painter.drawText(colX, y, "• Audio Mute / Unmute : Press [M] or [S]");
            y += 18;
            painter.drawText(colX, y, "• Pause : [ESC] or ⏸   •   Restart : [R]");
            y += 24;
        }
        else if (slideIndex == 2)
        {
            painter.setFont(secFont);
            painter.setPen(QColor(255, 183, 3));
            painter.drawText(colX, y, "POINTS & SPECIAL EGGS");
            y += 18;

            int lineH = 17;
            painter.setFont(boldBodyFont);
            painter.setPen(QColor(255, 255, 255));
            painter.drawText(colX, y, "[O] Regular");
            painter.setPen(QColor(6, 214, 160));
            painter.drawText(colX + 90, y, "+10 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Standard egg");
            y += lineH;

            painter.setFont(boldBodyFont);
            painter.setPen(QColor(255, 215, 0));
            painter.drawText(colX, y, "[*] Golden");
            painter.drawText(colX + 90, y, "+50 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Rare bonus");
            y += lineH;

            painter.setFont(boldBodyFont);
            painter.setPen(QColor(188, 108, 37));
            painter.drawText(colX, y, "[!] Bird Poop");
            painter.setPen(QColor(239, 71, 111));
            painter.drawText(colX + 90, y, "-1 Heart");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Makes sticky slow puddles!");
            y += lineH;

            painter.setFont(boldBodyFont);
            painter.setPen(QColor(46, 204, 113));
            painter.drawText(colX, y, "[+] Growth");
            painter.drawText(colX + 90, y, "+25 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Store in inventory, press [H] to grow!");
            y += lineH;

            painter.setFont(boldBodyFont);
            painter.setPen(QColor(0, 229, 255));
            painter.drawText(colX, y, "[^] Restore");
            painter.drawText(colX + 90, y, "+100 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Restores basket to max starting size!");
            y += lineH;

            painter.setFont(boldBodyFont);
            painter.setPen(QColor(255, 214, 10));
            painter.drawText(colX, y, "[♛] Trophy");
            painter.drawText(colX + 90, y, "+100 pts");
            painter.setFont(bodyFont);
            painter.setPen(QColor(156, 168, 184));
            painter.drawText(colX + 150, y, "Boss reward: +heart, wider basket!");
            y += 24;
        }
        else if (slideIndex == 3)
        {
            painter.setFont(secFont);
            painter.setPen(QColor(255, 112, 67));
            painter.drawText(colX, y, "BOSSES, WEATHER & HAZARDS");
            y += 18;

            painter.setFont(bodyFont);
            auto bullet = [&](const QColor &c, const QString &text) {
                painter.setPen(c);
                const QRect r(colX, y - 11, cardW - 60, 40);
                const QRect used = painter.boundingRect(r, Qt::TextWordWrap, text);
                painter.drawText(r, Qt::TextWordWrap, text);
                y += used.height() + 3;
            };
            bullet(QColor(255, 183, 3),  "⚔ A BOSS arrives at level 3, then every 10 levels. Catch its real eggs to drain its health bar; each volley hides one real egg among droppings.");
            bullet(QColor(255, 120, 120), "💢 Below 40% health the boss ENRAGES: faster volleys and a signature twist. Beat it for a bonus, a 👑 Trophy Egg and the next level.");
            bullet(QColor(190, 225, 255), "💨 WIND pushes eggs sideways. Watch the gust warning and lean into it.");
            bullet(QColor(150, 195, 240), "🌧 RAIN makes the basket slide, and mud forms on wet ground (heavy slow).");
            bullet(QColor(150, 235, 255), "❄ Night frost makes ICE patches (you slide, hard to stop). 🌫 Night FOG hides eggs up high.");
            y += 4;
        }

        // Slide pagination controls
        QFont navFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(navFont);
        painter.setPen(QColor(255, 209, 102));
        QString slideIndicator = QString("Page %1 of 4   •   Tap [◀ Prev] or [Next ▶] to change guide").arg(slideIndex + 1);
        painter.drawText(QRect(cardX, y, cardW, 20), Qt::AlignCenter, slideIndicator);

        painter.restore();

        // Draw scrollbar if content exceeds viewport
        if (maxScroll > 0)
        {
            drawScrollbar(painter, cardX + cardW - 12, contentTop + 4, 6, viewportH - 8, scrollY, maxScroll, totalContentH, QColor(255, 209, 102, 210));
        }
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
                         "Press [R] or [SPACE] to play again   •   [ESC] / [Q] to exit");
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        renderOnScreenControls(painter, engine, width, height);
    }
    else if (engine.getState() == GameState::PAUSED)
    {
        renderPauseMenu(painter, engine, width, height);
    }
    else if (engine.getState() == GameState::SETTINGS)
    {
        renderSettingsPage(painter, engine, audio, scale, width, height);
    }

    painter.end();

    // Screen shake (boss landings, big hits, rage): shift the finished frame a few pixels
    if (engine.getState() == GameState::PLAYING && engine.getScreenShakeTicks() > 0)
    {
        const int mag = std::max(1, std::min(engine.getScreenShakeMag(), (engine.getScreenShakeTicks() + 2) / 3 + 1)) * std::max(1, scale / 3);
        const unsigned h1 = static_cast<unsigned>(hudTick) * 2654435761u;
        const int dx = static_cast<int>((h1 >> 8) % (2 * mag + 1)) - mag;
        const int dy = static_cast<int>((h1 >> 17) % (2 * mag + 1)) - mag;
        QPixmap shaken(width, height);
        shaken.fill(QColor(12, 14, 22));
        QPainter sp(&shaken);
        sp.drawPixmap(dx, dy, pix);
        sp.end();
        return shaken;
    }
    return pix;
}

QRect GameRenderer::getPauseButtonRect(int width, int height)
{
    Q_UNUSED(height);
    const int sz = 34;
    return QRect(width - sz - 10, 10, sz, sz);
}

QRect GameRenderer::getSettingsBackButtonRect(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    return QRect(16, 12, 74, 28);
}

void GameRenderer::drawScrollbar(QPainter &painter, int trackX, int trackY, int trackW, int trackH, int scrollY, int maxScroll, int contentH, const QColor &accentColor)
{
    if (trackH <= 10) return;

    // Background track
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(22, 28, 42, 170));
    painter.drawRoundedRect(trackX, trackY, trackW, trackH, trackW / 2, trackW / 2);

    if (maxScroll <= 0 || contentH <= 0) return;

    // Draggable / visual thumb
    const int thumbH = std::clamp((trackH * trackH) / contentH, 20, trackH - 2);
    const int availableY = trackH - thumbH;
    const int thumbY = trackY + (scrollY * availableY) / maxScroll;

    painter.setBrush(accentColor);
    painter.drawRoundedRect(trackX + 1, thumbY, trackW - 2, thumbH, (trackW - 2) / 2, (trackW - 2) / 2);
}

QRect GameRenderer::getVirtualLeftButtonRect(int width, int height)
{
    const int btnW = std::clamp(width / 7, 68, 96);
    const int btnH = std::clamp(height / 7, 52, 72);
    return QRect(16, height - btnH - 16, btnW, btnH);
}

QRect GameRenderer::getVirtualRightButtonRect(int width, int height)
{
    const int btnW = std::clamp(width / 7, 68, 96);
    const int btnH = std::clamp(height / 7, 52, 72);
    return QRect(width - btnW - 16, height - btnH - 16, btnW, btnH);
}

QRect GameRenderer::getGrowthButtonRect(int width, int height)
{
    const int btnH = std::clamp(height / 7, 52, 72);
    const int sz = std::clamp(height / 9, 42, 54);
    return QRect(width - sz - 20, height - btnH - sz - 26, sz, sz);
}

QRect GameRenderer::getCompanionButtonRect(int width, int height)
{
    const int btnH = std::clamp(height / 7, 52, 72);
    const int sz = std::clamp(height / 9, 42, 54);
    return QRect(width - (sz * 2) - 30, height - btnH - sz - 26, sz, sz);
}

QRect GameRenderer::getDefendButtonRect(int width, int height)
{
    const int btnH = std::clamp(height / 7, 52, 72);
    const int sz = std::clamp(height / 9, 42, 54);
    const int defW = std::clamp(width / 7, 84, 115);
    return QRect(16, height - btnH - sz - 26, defW, sz);
}

void GameRenderer::renderOnScreenControls(QPainter &painter, const GameEngine &engine, int width, int height)
{
    // 1. Pause Button [ ⏸ ]
    const QRect pauseRect = getPauseButtonRect(width, height);
    painter.setPen(QPen(QColor(0, 229, 255, 180), 1.5));
    painter.setBrush(QColor(16, 24, 38, 180));
    painter.drawRoundedRect(pauseRect, 8, 8);
    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.setPen(QColor(0, 229, 255));
    painter.drawText(pauseRect, Qt::AlignCenter, "⏸");

    // 2. Growth Egg Button [ 🧺 H ] (visible when player holds growth eggs)
    if (engine.getGrowthEggCount() > 0)
    {
        const QRect hRect = getGrowthButtonRect(width, height);
        painter.setPen(QPen(QColor(46, 204, 113, 220), 2));
        painter.setBrush(QColor(22, 54, 38, 200));
        painter.drawRoundedRect(hRect, 10, 10);
        painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
        painter.setPen(QColor(46, 204, 113));
        painter.drawText(QRect(hRect.x(), hRect.y() + 4, hRect.width(), 18), Qt::AlignCenter, "🧺 H");
        painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(hRect.x(), hRect.y() + 22, hRect.width(), 16), Qt::AlignCenter,
                         QString("x%1").arg(engine.getGrowthEggCount()));
    }

    // 3. Companion Basket Button [ 🤖 C ] (visible in Hard mode when at least 2 eggs held)
    if (engine.getMode() == GameMode::HARD && engine.getGrowthEggCount() >= 2)
    {
        const QRect cRect = getCompanionButtonRect(width, height);
        painter.setPen(QPen(QColor(0, 229, 255, 220), 2));
        painter.setBrush(QColor(18, 44, 62, 200));
        painter.drawRoundedRect(cRect, 10, 10);
        painter.setFont(QFont("Segoe UI", 11, QFont::Bold));
        painter.setPen(QColor(0, 229, 255));
        painter.drawText(QRect(cRect.x(), cRect.y() + 4, cRect.width(), 18), Qt::AlignCenter, "🤖 C");
        painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter.setPen(QColor(255, 255, 255));
        painter.drawText(QRect(cRect.x(), cRect.y() + 22, cRect.width(), 16), Qt::AlignCenter, "SUMMON");
    }

    // 4. Defend Button [ 🛡️ DEFEND ]
    {
        const QRect defRect = getDefendButtonRect(width, height);
        const bool isDefending = engine.isBasketDefending();
        const bool canDefend = engine.getBasketDefendCooldown() <= 0;

        if (isDefending)
        {
            painter.setPen(QPen(QColor(0, 255, 255), 2.5));
            painter.setBrush(QColor(0, 180, 216, 220));
        }
        else if (canDefend)
        {
            painter.setPen(QPen(QColor(0, 229, 255, 200), 1.8));
            painter.setBrush(QColor(16, 32, 52, 200));
        }
        else
        {
            painter.setPen(QPen(QColor(70, 80, 100, 140), 1.0));
            painter.setBrush(QColor(16, 20, 28, 160));
        }
        painter.drawRoundedRect(defRect, 8, 8);

        QFont defFont = painter.font();
        defFont.setBold(true);
        defFont.setPixelSize(std::clamp(defRect.height() / 3, 11, 14));
        painter.setFont(defFont);
        painter.setPen(isDefending ? QColor(255, 255, 255) : (canDefend ? QColor(0, 229, 255) : QColor(140, 150, 170)));

        QString defText = isDefending ? "🛡️ SHIELD!" : (canDefend ? "🛡️ DEFEND" : "🛡️ WAIT");
        painter.drawText(defRect, Qt::AlignCenter, defText);
    }

    // 5. Mode-Specific Overlays
    const ControlMode mode = engine.getControlMode();
    if (mode == ControlMode::VIRTUAL_BUTTONS)
    {
        // Left Button [ ◀ ]
        const QRect leftRect = getVirtualLeftButtonRect(width, height);
        const bool leftDown = engine.isKeyLeftPressed();
        painter.setPen(QPen(leftDown ? QColor(0, 255, 255) : QColor(0, 229, 255, 160), leftDown ? 2.5 : 1.5));
        painter.setBrush(leftDown ? QColor(0, 200, 240, 130) : QColor(14, 22, 36, 160));
        painter.drawRoundedRect(leftRect, 14, 14);

        painter.setFont(QFont("Segoe UI", 22, QFont::Bold));
        painter.setPen(leftDown ? QColor(255, 255, 255) : QColor(0, 229, 255));
        painter.drawText(leftRect, Qt::AlignCenter, "◀");

        // Right Button [ ▶ ]
        const QRect rightRect = getVirtualRightButtonRect(width, height);
        const bool rightDown = engine.isKeyRightPressed();
        painter.setPen(QPen(rightDown ? QColor(0, 255, 255) : QColor(0, 229, 255, 160), rightDown ? 2.5 : 1.5));
        painter.setBrush(rightDown ? QColor(0, 200, 240, 130) : QColor(14, 22, 36, 160));
        painter.drawRoundedRect(rightRect, 14, 14);

        painter.setFont(QFont("Segoe UI", 22, QFont::Bold));
        painter.setPen(rightDown ? QColor(255, 255, 255) : QColor(0, 229, 255));
        painter.drawText(rightRect, Qt::AlignCenter, "▶");
    }
    else if (mode == ControlMode::GYROSCOPE)
    {
        // Sleek Tilt Bubble Indicator at bottom-center
        const int barW = 160;
        const int barH = 22;
        const int barX = (width - barW) / 2;
        const int barY = height - barH - 12;
        const QRect barRect(barX, barY, barW, barH);

        painter.setPen(QPen(QColor(60, 75, 105, 180), 1.5));
        painter.setBrush(QColor(12, 16, 26, 170));
        painter.drawRoundedRect(barRect, 11, 11);

        // Center line
        painter.setPen(QColor(100, 120, 150, 180));
        painter.drawLine(barX + barW / 2, barY + 3, barX + barW / 2, barY + barH - 3);

        // Tilt bubble
        const float tilt = engine.getGyroTilt();
        const int bubbleCenterX = barX + barW / 2 + static_cast<int>(tilt * (barW / 2 - 12));
        const int bubbleRadius = 8;
        painter.setPen(QPen(QColor(0, 255, 255), 1.5));
        painter.setBrush(QColor(0, 229, 255, 210));
        painter.drawEllipse(QPoint(bubbleCenterX, barY + barH / 2), bubbleRadius, bubbleRadius);

        // Subtle arrows
        painter.setFont(QFont("Segoe UI", 8, QFont::Bold));
        painter.setPen(tilt < -0.1f ? QColor(0, 255, 255) : QColor(90, 110, 135));
        painter.drawText(QRect(barX + 6, barY, 14, barH), Qt::AlignCenter, "◀");
        painter.setPen(tilt > 0.1f ? QColor(0, 255, 255) : QColor(90, 110, 135));
        painter.drawText(QRect(barX + barW - 20, barY, 14, barH), Qt::AlignCenter, "▶");
    }
}

void GameRenderer::renderPauseMenu(QPainter &painter, const GameEngine &engine, int width, int height)
{
    // Dim background overlay
    painter.fillRect(0, 0, width, height, QColor(10, 13, 22, 205));

    const int cardW = std::min(width - 40, 420);
    const int cardH = std::min(height - 40, 340);
    const int cardX = (width - cardW) / 2;
    const int cardY = (height - cardH) / 2;

    // Card frame
    painter.setPen(QPen(QColor(56, 70, 96), 2));
    painter.setBrush(QColor(18, 22, 34, 250));
    painter.drawRoundedRect(cardX, cardY, cardW, cardH, 12, 12);

    // Title
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 0, 0, 180));
    painter.drawText(QRect(cardX + 2, cardY + 10, cardW, 26), Qt::AlignCenter, "⏸ GAME PAUSED");
    painter.setPen(QColor(255, 209, 102));
    painter.drawText(QRect(cardX, cardY + 8, cardW, 26), Qt::AlignCenter, "⏸ GAME PAUSED");

    // Stats line
    QFont statsFont("Segoe UI", 9, QFont::Normal);
    painter.setFont(statsFont);
    painter.setPen(QColor(168, 199, 250));
    QString heartsStr;
    for (int i = 0; i < engine.getHearts(); ++i) heartsStr += "♥ ";
    QString statsText = QString("Score: %1   •   Level: %2   •   Hearts: %3")
                            .arg(engine.getScore())
                            .arg(engine.getLevel())
                            .arg(heartsStr.trimmed());
    painter.drawText(QRect(cardX, cardY + 34, cardW, 16), Qt::AlignCenter, statsText);

    // Accent line
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 209, 102, 180));
    painter.drawRoundedRect(cardX + 24, cardY + 52, cardW - 48, 1, 1, 1);

    // Scrollable Viewport
    const int contentTop = cardY + 56;
    const int viewportH = cardH - 64;
    const int totalContentH = 295;
    const int maxScroll = std::max(0, totalContentH - viewportH);
    const int scrollY = std::clamp(engine.getPauseScrollY(), 0, maxScroll);

    painter.save();
    painter.setClipRect(cardX + 4, contentTop, cardW - 8, viewportH);
    painter.translate(0, -scrollY);

    // Difficulty Mode Selector Pills
    const int modeBtnW = std::min(104, (cardW - 60) / 3);
    const int modeBtnH = 26;
    const int modeGap = 8;
    const int totalModeW = modeBtnW * 3 + modeGap * 2;
    const int startModeX = cardX + (cardW - totalModeW) / 2;
    const int modeBarY = contentTop + 6;

    QFont modeBtnFont("Segoe UI", 10, QFont::Bold);
    painter.setFont(modeBtnFont);

    // EASY button
    const bool isEasy = (engine.getMode() == GameMode::EASY);
    const QRect easyRect(startModeX, modeBarY, modeBtnW, modeBtnH);
    painter.setPen(QPen(isEasy ? QColor(46, 204, 113) : QColor(60, 75, 90), isEasy ? 2 : 1));
    painter.setBrush(isEasy ? QColor(25, 55, 38) : QColor(26, 32, 44));
    painter.drawRoundedRect(easyRect, 6, 6);
    painter.setPen(isEasy ? QColor(46, 204, 113) : QColor(160, 175, 190));
    painter.drawText(easyRect, Qt::AlignCenter, isEasy ? "● EASY" : "EASY");

    // MEDIUM button
    const bool isMed = (engine.getMode() == GameMode::MEDIUM);
    const QRect medRect(startModeX + modeBtnW + modeGap, modeBarY, modeBtnW, modeBtnH);
    painter.setPen(QPen(isMed ? QColor(255, 183, 3) : QColor(60, 75, 90), isMed ? 2 : 1));
    painter.setBrush(isMed ? QColor(55, 45, 20) : QColor(26, 32, 44));
    painter.drawRoundedRect(medRect, 6, 6);
    painter.setPen(isMed ? QColor(255, 183, 3) : QColor(160, 175, 190));
    painter.drawText(medRect, Qt::AlignCenter, isMed ? "● MEDIUM" : "MEDIUM");

    // HARD button
    const bool isHard = (engine.getMode() == GameMode::HARD);
    const QRect hardRect(startModeX + (modeBtnW + modeGap) * 2, modeBarY, modeBtnW, modeBtnH);
    painter.setPen(QPen(isHard ? QColor(239, 71, 111) : QColor(60, 75, 90), isHard ? 2 : 1));
    painter.setBrush(isHard ? QColor(60, 25, 35) : QColor(26, 32, 44));
    painter.drawRoundedRect(hardRect, 6, 6);
    painter.setPen(isHard ? QColor(239, 71, 111) : QColor(160, 175, 190));
    painter.drawText(hardRect, Qt::AlignCenter, isHard ? "● HARD" : "HARD");

    // Mode Brief Subtext
    QFont modeSumFont("Segoe UI", 8, QFont::Normal);
    painter.setFont(modeSumFont);
    QString modeSummary;
    if (isEasy)
    {
        modeSummary = "✓ Calm skies  •  1 starting bird  •  Standard drops";
    }
    else if (isMed)
    {
        modeSummary = "⚡ 2 starting birds  •  Faster scaling  •  Sticky poop";
    }
    else
    {
        modeSummary = "🔥 2 starting birds  •  Fierce speed  •  Companion basket";
    }
    painter.setPen(QColor(168, 199, 250));
    painter.drawText(QRect(cardX, modeBarY + 29, cardW, 16), Qt::AlignCenter, modeSummary);

    // Action Buttons
    const int btnW = cardW - 70;
    const int btnH = 34;
    const int btnGap = 8;
    const int btnX = cardX + (cardW - btnW) / 2;
    const int startY = modeBarY + 50;

    QFont btnFont("Segoe UI", 11, QFont::Bold);
    painter.setFont(btnFont);

    // 1. Resume Button
    const QRect btnResume(btnX, startY, btnW, btnH);
    painter.setPen(QPen(QColor(46, 204, 113), 2));
    painter.setBrush(QColor(22, 54, 38));
    painter.drawRoundedRect(btnResume, 8, 8);
    painter.setPen(QColor(46, 204, 113));
    painter.drawText(btnResume, Qt::AlignCenter, "▶   Resume Game   [ESC]");

    // 2. Settings Button
    const QRect btnSettings(btnX, startY + (btnH + btnGap), btnW, btnH);
    painter.setPen(QPen(QColor(0, 229, 255), 2));
    painter.setBrush(QColor(18, 44, 62));
    painter.drawRoundedRect(btnSettings, 8, 8);
    painter.setPen(QColor(0, 229, 255));
    painter.drawText(btnSettings, Qt::AlignCenter, "⚙   Game Settings");

    // 3. Restart Button
    const QRect btnRestart(btnX, startY + (btnH + btnGap) * 2, btnW, btnH);
    painter.setPen(QPen(QColor(255, 183, 3), 2));
    painter.setBrush(QColor(52, 40, 18));
    painter.drawRoundedRect(btnRestart, 8, 8);
    painter.setPen(QColor(255, 183, 3));
    painter.drawText(btnRestart, Qt::AlignCenter, "↻   Restart Run   [R]");

    // 4. Main Menu Button
    const QRect btnMainMenu(btnX, startY + (btnH + btnGap) * 3, btnW, btnH);
    painter.setPen(QPen(QColor(239, 71, 111), 2));
    painter.setBrush(QColor(56, 24, 36));
    painter.drawRoundedRect(btnMainMenu, 8, 8);
    painter.setPen(QColor(239, 71, 111));
    painter.drawText(btnMainMenu, Qt::AlignCenter, "⌂   Main Menu (Starting Page)");

    // 5. Exit Game Button
    const QRect btnExit(btnX, startY + (btnH + btnGap) * 4, btnW, btnH);
    painter.setPen(QPen(QColor(230, 57, 70), 2));
    painter.setBrush(QColor(50, 18, 24));
    painter.drawRoundedRect(btnExit, 8, 8);
    painter.setPen(QColor(230, 57, 70));
    painter.drawText(btnExit, Qt::AlignCenter, "✕   Exit Game");

    // Hint
    QFont hintFont("Segoe UI", 9, QFont::Normal);
    painter.setFont(hintFont);
    painter.setPen(QColor(139, 155, 180));
    painter.drawText(QRect(cardX, startY + (btnH + btnGap) * 5 + 4, cardW, 18), Qt::AlignCenter,
                     "Press [ESC] to resume  •  [M] Main Menu  •  [R] Restart  •  [Q] Exit");

    painter.restore();

    // Draw scrollbar if content exceeds viewport
    if (maxScroll > 0)
    {
        drawScrollbar(painter, cardX + cardW - 12, contentTop + 4, 6, viewportH - 8, scrollY, maxScroll, totalContentH, QColor(255, 209, 102, 210));
    }
}

void GameRenderer::renderSettingsPage(QPainter &painter, const GameEngine &engine, const AudioManager *audio, int scale, int width, int height)
{
    // Dim background overlay
    painter.fillRect(0, 0, width, height, QColor(8, 12, 20, 225));

    // Small Back/Done Button OUTSIDE the page frame (at top left)
    const QRect btnBack = getSettingsBackButtonRect(width, height);
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.setPen(QPen(QColor(0, 229, 255), 1.5));
    painter.setBrush(QColor(18, 44, 62, 230));
    painter.drawRoundedRect(btnBack, 6, 6);
    painter.setPen(QColor(0, 229, 255));
    painter.drawText(btnBack, Qt::AlignCenter, "◀ Back");

    const int cardW = std::min(width - 40, 560);
    const int cardH = std::min(height - 40, 350);
    const int cardX = (width - cardW) / 2;
    const int cardY = (height - cardH) / 2;

    // Card frame
    painter.setPen(QPen(QColor(48, 70, 105), 2));
    painter.setBrush(QColor(18, 22, 34, 252));
    painter.drawRoundedRect(cardX, cardY, cardW, cardH, 12, 12);

    // Accent line
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 229, 255));
    painter.drawRoundedRect(cardX + 24, cardY + 46, cardW - 48, 2, 1, 1);

    // Title
    QFont titleFont("Segoe UI", 18, QFont::Bold);
    painter.setFont(titleFont);
    painter.setPen(QColor(0, 0, 0, 180));
    painter.drawText(QRect(cardX + 2, cardY + 12, cardW, 28), Qt::AlignCenter, "⚙ GAME SETTINGS");
    painter.setPen(QColor(0, 229, 255));
    painter.drawText(QRect(cardX, cardY + 10, cardW, 28), Qt::AlignCenter, "⚙ GAME SETTINGS");

    // Scrollable Viewport
    const int contentTop = cardY + 52;
    const int viewportH = cardH - 60;
    const int totalContentH = 325;
    const int maxScroll = std::max(0, totalContentH - viewportH);
    const int scrollY = std::clamp(engine.getSettingsScrollY(), 0, maxScroll);

    painter.save();
    painter.setClipRect(cardX + 4, contentTop, cardW - 8, viewportH);
    painter.translate(0, -scrollY);

    const int rowLeftX = cardX + 30;
    QFont lblFont("Segoe UI", 10, QFont::Bold);
    QFont btnFont("Segoe UI", 9, QFont::Bold);

    // --- ROW 1: Music (BGM) ---
    const int y1 = contentTop + 8;
    painter.setFont(lblFont);
    painter.setPen(QColor(224, 230, 237));
    painter.drawText(QRect(rowLeftX, y1, 120, 26), Qt::AlignLeft | Qt::AlignVCenter, "Music (BGM):");

    const bool bgmMuted = audio ? audio->isBGMMuted() : false;
    const float bgmVol = audio ? audio->getBGMVolume() : 0.38f;

    // Mute toggle
    painter.setFont(btnFont);
    const QRect bgmMuteBtn(rowLeftX + 120, y1, 60, 26);
    painter.setPen(QPen(bgmMuted ? QColor(239, 71, 111) : QColor(46, 204, 113), 1));
    painter.setBrush(bgmMuted ? QColor(56, 24, 36) : QColor(22, 54, 38));
    painter.drawRoundedRect(bgmMuteBtn, 6, 6);
    painter.setPen(bgmMuted ? QColor(239, 71, 111) : QColor(46, 204, 113));
    painter.drawText(bgmMuteBtn, Qt::AlignCenter, bgmMuted ? "MUTED" : "ON");

    // Volume Presets (0%, 25%, 50%, 75%, 100%)
    const int volStartX = rowLeftX + 190;
    const int volBtnW = 48;
    const int volBtnH = 26;
    const int volGap = 6;
    const float volValues[5] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
    const QString volLabels[5] = {"0%", "25%", "50%", "75%", "100%"};

    for (int i = 0; i < 5; ++i)
    {
        const QRect vb(volStartX + i * (volBtnW + volGap), y1, volBtnW, volBtnH);
        const bool active = (!bgmMuted && std::abs(bgmVol - volValues[i]) < 0.13f);
        painter.setPen(QPen(active ? QColor(255, 209, 102) : QColor(60, 75, 95), active ? 2 : 1));
        painter.setBrush(active ? QColor(55, 45, 20) : QColor(26, 32, 44));
        painter.drawRoundedRect(vb, 6, 6);
        painter.setPen(active ? QColor(255, 209, 102) : QColor(160, 175, 195));
        painter.drawText(vb, Qt::AlignCenter, volLabels[i]);
    }

    // --- ROW 2: Sound Effects (SFX) ---
    const int y2 = contentTop + 48;
    painter.setFont(lblFont);
    painter.setPen(QColor(224, 230, 237));
    painter.drawText(QRect(rowLeftX, y2, 120, 26), Qt::AlignLeft | Qt::AlignVCenter, "Sound FX (SFX):");

    const bool sfxMuted = audio ? audio->isSFXMuted() : false;
    const float sfxVol = audio ? audio->getSFXVolume() : 0.75f;

    // SFX Mute toggle
    painter.setFont(btnFont);
    const QRect sfxMuteBtn(rowLeftX + 120, y2, 60, 26);
    painter.setPen(QPen(sfxMuted ? QColor(239, 71, 111) : QColor(46, 204, 113), 1));
    painter.setBrush(sfxMuted ? QColor(56, 24, 36) : QColor(22, 54, 38));
    painter.drawRoundedRect(sfxMuteBtn, 6, 6);
    painter.setPen(sfxMuted ? QColor(239, 71, 111) : QColor(46, 204, 113));
    painter.drawText(sfxMuteBtn, Qt::AlignCenter, sfxMuted ? "MUTED" : "ON");

    for (int i = 0; i < 5; ++i)
    {
        const QRect vb(volStartX + i * (volBtnW + volGap), y2, volBtnW, volBtnH);
        const bool active = (!sfxMuted && std::abs(sfxVol - volValues[i]) < 0.13f);
        painter.setPen(QPen(active ? QColor(255, 209, 102) : QColor(60, 75, 95), active ? 2 : 1));
        painter.setBrush(active ? QColor(55, 45, 20) : QColor(26, 32, 44));
        painter.drawRoundedRect(vb, 6, 6);
        painter.setPen(active ? QColor(255, 209, 102) : QColor(160, 175, 195));
        painter.drawText(vb, Qt::AlignCenter, volLabels[i]);
    }

    // --- ROW 3: Input Controls ---
    const int y3 = contentTop + 88;
    painter.setFont(lblFont);
    painter.setPen(QColor(224, 230, 237));
    painter.drawText(QRect(rowLeftX, y3, 110, 26), Qt::AlignLeft | Qt::AlignVCenter, "Controls:");

    const ControlMode curMode = engine.getControlMode();
    const int ctlStartX = rowLeftX + 110;
    const int ctlBtnW = 82;
    const int ctlBtnH = 26;
    const int ctlGap = 6;

    const struct {
        ControlMode mode;
        const char *label;
        QColor activeColor;
        QColor activeBg;
    } ctlOptions[4] = {
        {ControlMode::SCREEN_TOUCH,    "📱 Touch",       QColor(0, 229, 255),   QColor(18, 44, 62)},
        {ControlMode::VIRTUAL_BUTTONS, "🎮 Buttons",     QColor(46, 204, 113),  QColor(22, 54, 38)},
        {ControlMode::GYROSCOPE,       "🔄 Gyro",        QColor(199, 125, 255), QColor(45, 25, 55)},
        {ControlMode::MOUSE_KEYBOARD,  "🖱 Mouse/Keys",  QColor(255, 209, 102), QColor(55, 45, 20)}
    };

    painter.setFont(btnFont);
    for (int i = 0; i < 4; ++i)
    {
        const QRect cb(ctlStartX + i * (ctlBtnW + ctlGap), y3, ctlBtnW, ctlBtnH);
        const bool active = (curMode == ctlOptions[i].mode);
        painter.setPen(QPen(active ? ctlOptions[i].activeColor : QColor(60, 75, 95), active ? 2 : 1));
        painter.setBrush(active ? ctlOptions[i].activeBg : QColor(26, 32, 44));
        painter.drawRoundedRect(cb, 6, 6);
        painter.setPen(active ? ctlOptions[i].activeColor : QColor(160, 175, 195));
        painter.drawText(cb, Qt::AlignCenter, ctlOptions[i].label);
    }

    // --- ROW 4: Thunder Screen Flash (Photosensitivity) ---
    const int y4 = contentTop + 128;
    painter.setFont(lblFont);
    painter.setPen(QColor(224, 230, 237));
    painter.drawText(QRect(rowLeftX, y4, 120, 26), Qt::AlignLeft | Qt::AlignVCenter, "Thunder Flash:");

    const bool flashOn = engine.isLightningFlashEnabled();
    const QRect btnFlashOn(rowLeftX + 120, y4, 105, 26);
    const QRect btnFlashOff(rowLeftX + 235, y4, 150, 26);

    painter.setFont(btnFont);
    painter.setPen(QPen(flashOn ? QColor(46, 204, 113) : QColor(60, 75, 95), flashOn ? 2 : 1));
    painter.setBrush(flashOn ? QColor(22, 54, 38) : QColor(26, 32, 44));
    painter.drawRoundedRect(btnFlashOn, 6, 6);
    painter.setPen(flashOn ? QColor(46, 204, 113) : QColor(160, 175, 195));
    painter.drawText(btnFlashOn, Qt::AlignCenter, "⚡ Flash ON");

    painter.setPen(QPen(!flashOn ? QColor(255, 183, 3) : QColor(60, 75, 95), !flashOn ? 2 : 1));
    painter.setBrush(!flashOn ? QColor(55, 45, 20) : QColor(26, 32, 44));
    painter.drawRoundedRect(btnFlashOff, 6, 6);
    painter.setPen(!flashOn ? QColor(255, 183, 3) : QColor(160, 175, 195));
    painter.drawText(btnFlashOff, Qt::AlignCenter, "🛡 OFF (Reduced)");

    // --- ROW 5: Pixel Art Scale ---
    const int y5 = contentTop + 168;
    painter.setFont(lblFont);
    painter.setPen(QColor(224, 230, 237));
    painter.drawText(QRect(rowLeftX, y5, 120, 26), Qt::AlignLeft | Qt::AlignVCenter, "Graphics Scale:");

    const int scales[4] = {3, 4, 5, 6};
    const QString scaleLabels[4] = {"3x", "4x", "5x", "6x"};
    for (int i = 0; i < 4; ++i)
    {
        const QRect sb(rowLeftX + 120 + i * 62, y5, 52, 26);
        const bool active = (scale == scales[i]);
        painter.setPen(QPen(active ? QColor(199, 125, 255) : QColor(60, 75, 95), active ? 2 : 1));
        painter.setBrush(active ? QColor(45, 25, 55) : QColor(26, 32, 44));
        painter.drawRoundedRect(sb, 6, 6);
        painter.setPen(active ? QColor(199, 125, 255) : QColor(160, 175, 195));
        painter.drawText(sb, Qt::AlignCenter, scaleLabels[i]);
    }

    // --- Keybinds Overview Panel ---
    const int y6 = contentTop + 208;
    const QRect keyBox(rowLeftX, y6, cardW - 70, 74);
    painter.setPen(QPen(QColor(42, 54, 76), 1));
    painter.setBrush(QColor(14, 18, 28, 200));
    painter.drawRoundedRect(keyBox, 8, 8);

    QFont keyTitleFont("Segoe UI", 9, QFont::Bold);
    painter.setFont(keyTitleFont);
    painter.setPen(QColor(255, 209, 102));
    painter.drawText(QRect(rowLeftX + 10, y6 + 5, cardW - 90, 16), Qt::AlignLeft, "QUICK CONTROLS REFERENCE:");

    QFont keyBodyFont("Segoe UI", 8, QFont::Normal);
    painter.setFont(keyBodyFont);
    painter.setPen(QColor(180, 195, 215));

    QString moveTip;
    if (curMode == ControlMode::SCREEN_TOUCH)
    {
        moveTip = "• Move: Touch or drag anywhere on screen to guide basket (speed-capped)";
    }
    else if (curMode == ControlMode::VIRTUAL_BUTTONS)
    {
        moveTip = "• Move: Touch [ ◀ ] and [ ▶ ] on-screen buttons on bottom corners";
    }
    else if (curMode == ControlMode::GYROSCOPE)
    {
        moveTip = "• Move: Tilt phone left / right to steer basket; hold flat to stop";
    }
    else
    {
        moveTip = "• Move: Mouse Cursor OR [A / D] / [Left / Right Arrow] Keys (speed-capped)";
    }

    painter.drawText(QRect(rowLeftX + 10, y6 + 24, cardW - 90, 16), Qt::AlignLeft, moveTip);
    painter.drawText(QRect(rowLeftX + 10, y6 + 42, cardW - 90, 16), Qt::AlignLeft,
                     "• [H] or 🧺 Button: Growth Egg  •  [C] or 🤖 Button: Companion (Hard)  •  [ESC] or ⏸: Pause");
    painter.drawText(QRect(rowLeftX + 10, y6 + 58, cardW - 90, 16), Qt::AlignLeft,
                     "• Scroll: Touch / Drag up and down or use Mouse Wheel to scroll settings");

    painter.restore();

    // Draw scrollbar if content exceeds viewport
    if (maxScroll > 0)
    {
        drawScrollbar(painter, cardX + cardW - 12, contentTop + 4, 6, viewportH - 8, scrollY, maxScroll, totalContentH, QColor(0, 229, 255, 210));
    }
}
