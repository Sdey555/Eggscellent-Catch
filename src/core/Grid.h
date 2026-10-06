#ifndef GRID_H
#define GRID_H

#include <cmath>

// -------------------------------------------------------------
// Core Raster Grid Logic (Coordinate Math, Scale, Origin, Pan)
// -------------------------------------------------------------
class MyGrid
{
public:
    MyGrid();

    void setDimensions(int w, int h);
    void setScale(int newScale);

    int getScale() const { return scale; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getOriginX() const { return originX; }
    int getOriginY() const { return originY; }
    int getPanOffsetX() const { return panOffsetX; }
    int getPanOffsetY() const { return panOffsetY; }

    void pan(int dx, int dy);
    void resetPan();

    int screenToMathX(int screenX) const;
    int screenToMathY(int screenY) const;
    int mathToScreenX(int mathX) const;
    int mathToScreenY(int mathY) const;

private:
    int width{0};
    int height{0};
    int scale{4};
    int originX{0};
    int originY{0};
    int panOffsetX{0};
    int panOffsetY{0};

    void calculateOrigin();
};

#endif // GRID_H
