#ifndef PIXEL_H
#define PIXEL_H

#include <QColor>
#include <unordered_map>

// -------------------------------------------------------------
// Individual Raster Pixel Representation
// -------------------------------------------------------------
class MyPix
{
public:
    explicit MyPix(int x = 0, int y = 0, const QColor &color = QColor(255, 255, 255));

    int getX() const { return x; }
    int getY() const { return y; }
    QColor getColor() const { return color; }
    void setColor(const QColor &c) { color = c; }

private:
    int x;
    int y;
    QColor color;
};

// -------------------------------------------------------------
// 2D Raster Pixel Map for Dynamic Frame Buffering
// -------------------------------------------------------------
class MyPixels
{
public:
    static inline long long makeKey(int x, int y)
    {
        return (static_cast<long long>(x) << 32) | static_cast<unsigned int>(y);
    }

    void setPixel(int x, int y, const QColor &color);
    void clear();
    bool hasPixel(int x, int y) const;
    bool getPixelColor(int x, int y, QColor &outColor) const;
    const std::unordered_map<long long, QColor>& getPixelMap() const { return pixelMap; }

private:
    std::unordered_map<long long, QColor> pixelMap;
};

#endif // PIXEL_H
