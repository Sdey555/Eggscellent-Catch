#ifndef GAMERENDERER_H
#define GAMERENDERER_H

#include <QPixmap>
#include <QPainter>
#include <QColor>

class GameEngine;
class MyGrid;
class MyPixels;

class GameRenderer
{
public:
    GameRenderer();

    // Renders the entire game state into a QPixmap of the given size
    QPixmap renderFrame(const GameEngine &engine,
                        MyGrid &grid,
                        MyPixels &pixels,
                        int width,
                        int height,
                        int scale,
                        int hudTick);

    // Rasterization helper functions
    static void paintCell(QPainter &painter,
                          const MyGrid &grid,
                          int scale,
                          int width,
                          int height,
                          int mathX,
                          int mathY,
                          const QColor &color);

    static void rasterizeGround(MyPixels &pixels, int minX, int maxX, int gY, int minY);
    static void rasterizePixelHearts(MyPixels &pixels, int startX, int startY, int currentHearts, int maxHearts);
    static void rasterizeLevelBar(MyPixels &pixels, int minX, int maxX, int topY, int level, int score, int hudTick);

    // Pure Pixelated Background layers (zero smooth drawing, all snapped to raster cells)
    static void renderPixelSky(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                               int minMathX, int maxMathX, int minMathY, int maxMathY, int groundMathY);
    static void renderPixelClouds(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                  int minMathX, int maxMathX, int maxMathY, int hudTick);
    static void renderPixelHills(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                 int minMathX, int maxMathX, int groundMathY);
    static void renderPixelFarmhouse(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                     int groundMathY, int hudTick);
    static void renderPixelFence(QPainter &painter, const MyGrid &grid, int scale, int width, int height,
                                 int minMathX, int maxMathX, int groundMathY);
};

#endif // GAMERENDERER_H
