#ifndef PIXELFONT_H
#define PIXELFONT_H

#include <QString>
#include <QColor>

class MyPixels;

class PixelFont
{
public:
    static int drawText(MyPixels &pixels, int x, int topY, const QString &text, const QColor &color);
    static int textWidth(const QString &text);
};

#endif // PIXELFONT_H
