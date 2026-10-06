#ifndef BIRD_H
#define BIRD_H

#include <QColor>
#include "GameTypes.h"

class MyPixels;

struct Bird {
    double mathX = 0.0;
    double mathY = 0.0;
    double speed = 0.28;
    int direction = 1; // +1 right, -1 left
    int wingFrame = 0;
    int wingTick = 0;
    QColor bodyColor;
    QColor wingColor;
    QColor bellyColor;
    bool isLaying = false;
    int layingCountdown = 0;
    EggType pendingEggType = EggType::REGULAR;
};

void rasterizeBird(MyPixels &pixels, const Bird &b);

#endif // BIRD_H
