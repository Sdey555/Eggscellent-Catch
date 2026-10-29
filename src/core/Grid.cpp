#include "Grid.h"
#include <algorithm>

MyGrid::MyGrid()
    : width(0),
      height(0),
      scale(4),
      originX(0),
      originY(0),
      panOffsetX(0),
      panOffsetY(0)
{
}

void MyGrid::setDimensions(int w, int h)
{
    width = w;
    height = h;
    calculateOrigin();
}

void MyGrid::setScale(int newScale)
{
    scale = newScale;
    calculateOrigin();
}

void MyGrid::pan(int dx, int dy)
{
    panOffsetX += dx;
    panOffsetY += dy;
    calculateOrigin();
}

void MyGrid::resetPan()
{
    panOffsetX = 0;
    panOffsetY = 0;
    calculateOrigin();
}

int MyGrid::screenToMathX(int screenX) const
{
    if (scale <= 0) return 0;
    return static_cast<int>(
        std::floor(static_cast<double>(screenX - originX) / scale));
}

int MyGrid::screenToMathY(int screenY) const
{
    if (scale <= 0) return 0;
    return static_cast<int>(
        std::floor(static_cast<double>(originY - screenY - 1.0) / scale));
}

int MyGrid::mathToScreenX(int mathX) const
{
    return originX + mathX * scale;
}

int MyGrid::mathToScreenY(int mathY) const
{
    return originY - (mathY + 1) * scale;
}

void MyGrid::calculateOrigin()
{
    if (scale <= 0)
        return;
    int baseOriginX = static_cast<int>(std::round((width / 2.0) / scale) * scale);
    int baseOriginY = static_cast<int>(std::round((height / 2.0) / scale) * scale);
    originX = baseOriginX + panOffsetX;
    originY = baseOriginY + panOffsetY;
}
