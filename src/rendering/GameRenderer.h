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
};

#endif // GAMERENDERER_H
