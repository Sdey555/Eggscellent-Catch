#include "Pixel.h"

MyPix::MyPix(int x, int y, const QColor &color)
    : x(x), y(y), color(color)
{
}

void MyPixels::setPixel(int x, int y, const QColor &color)
{
    pixelMap[makeKey(x, y)] = color;
}

void MyPixels::clear()
{
    pixelMap.clear();
}

bool MyPixels::hasPixel(int x, int y) const
{
    return pixelMap.find(makeKey(x, y)) != pixelMap.end();
}

bool MyPixels::getPixelColor(int x, int y, QColor &outColor) const
{
    auto it = pixelMap.find(makeKey(x, y));
    if (it != pixelMap.end())
    {
        outColor = it->second;
        return true;
    }
    return false;
}
