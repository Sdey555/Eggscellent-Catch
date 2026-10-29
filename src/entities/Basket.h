#ifndef BASKET_H
#define BASKET_H

#include "GameTypes.h"

class MyPixels;

struct Basket {
    double mathX = 0.0;
    int mathY = 0;
    int halfWidth = 12;   // Larger default width (19 pixels total)
    int height = 7;      // Larger height (7 rows)
    double speed = 1.6;
    bool isDefending = false;
};

void rasterizeBasket(MyPixels &pixels, const Basket &b);
void rasterizeCompanionBasket(MyPixels &pixels, const CompanionBasket &comp);

#endif // BASKET_H
