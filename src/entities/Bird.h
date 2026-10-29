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
    bool isLegendary = false;
    int animTick = 0;

    // Cloud banks: gliding into drifting cloud & dropping eggs from underside
    bool inCloud = false;
    double cloudUnderbellyY = 0.0;
    int cloudIndex = -1;

    // Fog dive at night: dips into fog and emerges as dark silhouette with faint eye glint
    bool inFogDive = false;
    int fogDiveTicks = 0;
    double fogDiveBaseY = 0.0;
    bool isSilhouette = false;

    // Swoop / Dive Attack against the basket (From Level 1 onwards)
    enum class AttackState {
        NONE,
        TELEGRAPH, // Pre-dive targeting & warning
        DIVE,      // Fast swoop dive toward basket
        RECOVER    // Swooping back up to cruising altitude
    };
    AttackState attackState = AttackState::NONE;
    int attackTicks = 0;
    double attackStartX = 0.0;
    double attackStartY = 0.0;
    double attackTargetX = 0.0;
    double attackTargetY = 0.0;
};

void rasterizeBird(MyPixels &pixels, const Bird &b);
void rasterizeLegendaryBird(MyPixels &pixels, const Bird &b);

#endif // BIRD_H
