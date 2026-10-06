#ifndef FALLINGEGG_H
#define FALLINGEGG_H

#include "GameTypes.h"

class MyPixels;

struct FallingEgg {
    double mathX = 0.0;
    double mathY = 0.0;
    double vy = 0.08;          // current downward velocity
    double gravity = 0.015;    // velocity increment for gravity effect
    double maxSpeed = 1.6;     // terminal velocity
    EggType type = EggType::REGULAR;
    bool active = false;
    int animTick = 0;
};

void rasterizeEgg(MyPixels &pixels, const FallingEgg &egg);

#endif // FALLINGEGG_H
