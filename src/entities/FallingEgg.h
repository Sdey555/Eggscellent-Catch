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
    double vx = 0.0;           // horizontal drift (boss volleys fan out; wind adds on top of this)
    bool bossShot = false;     // Laid by a boss: catching it damages the boss
    bool telegraphed = false;  // Real boss egg beacon & ground landing indicator
    PatchType patchOnLand = PatchType::SLIME; // Hazard a boss poop leaves where it lands
    bool leavesPatch = false;  // Force a patch on landing (boss poop), regardless of mode
};

void rasterizeEgg(MyPixels &pixels, const FallingEgg &egg);

#endif // FALLINGEGG_H
