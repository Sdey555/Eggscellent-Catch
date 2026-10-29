#ifndef PARTICLE_H
#define PARTICLE_H

#include <QColor>
#include <QString>

struct GameParticle {
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    QColor color;
    int life = 0;
    int maxLife = 0;
};

struct FloatingText {
    double x = 0.0;
    double y = 0.0;
    QString text;
    QColor color;
    int life = 0;
};

#endif // PARTICLE_H
