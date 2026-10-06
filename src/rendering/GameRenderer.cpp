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
        // Top lush grass
        pixels.setPixel(x, gY, ((x % 3 == 0) ? QColor(88, 214, 141) : QColor(46, 204, 113)));
        // Sub-grass
        pixels.setPixel(x, gY - 1, QColor(39, 174, 96));

        // Earth layers
        for (int y = gY - 2; y >= minY; --y)
        {
            if ((x * 7 + y * 13) % 11 == 0)
            {
                pixels.setPixel(x, y, QColor(121, 85, 72)); // soil speckle
            }
            else if (y > gY - 5)
            {
                pixels.setPixel(x, y, QColor(93, 64, 55));
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
        const QColor heartCol = active ? QColor(239, 71, 111) : QColor(50, 58, 76);

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
    const QColor emptyCol(30, 34, 50);
    const QColor notchCol(22, 25, 38);
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
    pix.fill(QColor(14, 17, 24)); // Dark arcade night canvas
    QPainter painter(&pix);

    grid.setDimensions(width, height);
    grid.setScale(scale);

    // 1. Clear raster pixel buffer and rasterize all game entities
    pixels.clear();

    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);
    const int minMathY = grid.screenToMathY(height);
    const int maxMathY = grid.screenToMathY(0);

    // Rasterize Ground
    rasterizeGround(pixels, minMathX, maxMathX, engine.getGroundMathY(), minMathY);

    // Rasterize Birds
    for (const auto &bird : engine.getBirds())
    {
        rasterizeBird(pixels, bird);
    }

    // Rasterize Basket
    rasterizeBasket(pixels, engine.getBasket());

    // Rasterize Falling Egg
    const FallingEgg &egg = engine.getFallingEgg();
    if (egg.active)
    {
        rasterizeEgg(pixels, egg);
    }

    // Rasterize Hearts in canvas top-left
    rasterizePixelHearts(pixels, minMathX + 3, maxMathY - 3, engine.getHearts(), 3);

    // Rasterize Level Progress Bar in the top-right
    rasterizeLevelBar(pixels, maxMathX - 40, maxMathX - 3, maxMathY - 2, engine.getLevel(), engine.getScore(), hudTick);

    // 2. PASS 1: Pixel Glow Aura Pass (glowing aura radiating beyond cells without any grid lines)
    const int glowRadius = (scale <= 3 ? 1 : (scale <= 6 ? 2 : 3));
    const int glowAlpha = (scale <= 3 ? 32 : 45);

    // Special glowing aura for Golden Egg
    if (egg.active && egg.type == EggType::GOLDEN)
    {
        const int ex = grid.mathToScreenX(static_cast<int>(std::round(egg.mathX)));
        const int ey = grid.mathToScreenY(static_cast<int>(std::round(egg.mathY)));
        const int auraR = scale * 7;
        QRadialGradient goldGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
        goldGlow.setColorAt(0.0, QColor(255, 215, 0, 75));
        goldGlow.setColorAt(0.4, QColor(255, 190, 0, 28));
        goldGlow.setColorAt(1.0, QColor(255, 180, 0, 0));
        painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, goldGlow);
    }
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

    // 3. PASS 2: Pixel Core Pass (draws crisp solid cells with inner phosphor brightness, no grid lines)
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

    // 4. Paint Floating Texts
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

    // 5. Draw Progress Bar Text below the bar
    const int level = engine.getLevel();
    const int score = engine.getScore();
    if (engine.getState() == GameState::PLAYING)
    {
        const bool maxed = level >= GameEngine::MAX_LEVEL;
        const int levelStart = (level - 1) * GameEngine::POINTS_PER_LEVEL;
        const int earned = maxed ? GameEngine::POINTS_PER_LEVEL : std::clamp(score - levelStart, 0, GameEngine::POINTS_PER_LEVEL);
        const int remaining = GameEngine::POINTS_PER_LEVEL - earned;
        const QString progressLabel = maxed ? QString("MAX LEVEL")
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

    // 6. Level-up banner (fades out)
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
                         "Faster eggs • More bombs • Smaller basket");
    }

    // 7. Overlays for Game Over and Paused
    if (engine.getState() == GameState::GAME_OVER)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 210));

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
                         "Click, press [SPACE] or [R], or click 'Restart' to play again!");
    }
    else if (engine.getState() == GameState::PAUSED)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 175));

        painter.setPen(QColor(255, 209, 102));
        QFont pauseFont("Segoe UI", 28, QFont::Bold);
        painter.setFont(pauseFont);
        painter.drawText(QRect(0, height / 2 - 50, width, 50), Qt::AlignCenter, "PAUSED");

        painter.setPen(QColor(224, 230, 237));
        QFont promptFont("Segoe UI", 13, QFont::Normal);
        painter.setFont(promptFont);
        painter.drawText(QRect(0, height / 2 + 15, width, 30), Qt::AlignCenter,
                         "Click, press [SPACE], or click 'Resume' to continue");
    }

    painter.end();
    return pix;
}
