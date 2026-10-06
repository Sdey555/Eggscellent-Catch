#ifndef BASKET_H
#define BASKET_H

class MyPixels;

struct Basket {
    double mathX = 0.0;
    int mathY = 0;
    int halfWidth = 6;
    int height = 5;
    double speed = 1.6;
};

void rasterizeBasket(MyPixels &pixels, const Basket &b);

#endif // BASKET_H
