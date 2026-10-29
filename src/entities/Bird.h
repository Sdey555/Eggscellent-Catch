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
};

void rasterizeBird(MyPixels &pixels, const Bird &b);
void rasterizeLegendaryBird(MyPixels &pixels, const Bird &b);

#endif // BIRD_H
