#include "Boss.h"
#include "Pixel.h"
#include <algorithm>
#include <cmath>

// -------------------------------------------------------------
// Boss table
// -------------------------------------------------------------
static const BossDef kBosses[BOSS_TIERS] = {
    // 3: COLONEL CLUCK - a furious rooster (first boss at level 3)
    { "COLONEL CLUCK", "The Sky Rooster",
      { QColor(60, 18, 22),   QColor(214, 69, 65),  QColor(244, 118, 96), QColor(150, 40, 45),
        QColor(168, 40, 58),  QColor(255, 196, 92), QColor(104, 22, 42),
        QColor(255, 214, 102), QColor(255, 183, 3), QColor(231, 60, 60),  QColor(255, 128, 110), QColor(255, 214, 10) },
      8, PAT_FAN, PAT_SWEEP, BossTrait::NONE, 92, 0.95 },

    // 20: STORM CONDOR - thunderbird, rage whips up crosswinds
    { "STORM CONDOR", "Thunderbird of the Gale",
      { QColor(10, 14, 24),   QColor(58, 80, 104),  QColor(104, 134, 168), QColor(30, 42, 60),
        QColor(44, 62, 84),   QColor(140, 190, 235), QColor(20, 30, 46),
        QColor(189, 214, 235), QColor(255, 183, 3), QColor(255, 235, 59), QColor(255, 250, 170), QColor(0, 229, 255) },
      14, PAT_FAN | PAT_SWEEP, PAT_BURST | PAT_RAIN, BossTrait::GALE, 70, 1.1 },

    // 30: MAGPIE KING - hoarder, its droppings are mud bombs
    { "MAGPIE KING", "Lord of the Hoard",
      { QColor(8, 8, 16),     QColor(34, 36, 56),   QColor(78, 84, 124),  QColor(16, 17, 28),
        QColor(30, 54, 92),   QColor(0, 188, 170),  QColor(16, 26, 52),
        QColor(246, 246, 252), QColor(255, 160, 30), QColor(255, 200, 40), QColor(255, 240, 150), QColor(255, 215, 0) },
      18, PAT_FAN | PAT_BURST | PAT_SWEEP, PAT_RAIN, BossTrait::MUD, 64, 1.15 },

    // 40: FROST ROC - its droppings freeze into ice
    { "FROST ROC", "Glacier Wing",
      { QColor(18, 40, 72),   QColor(150, 200, 232), QColor(226, 246, 255), QColor(84, 134, 184),
        QColor(116, 172, 220), QColor(232, 250, 255), QColor(56, 98, 160),
        QColor(250, 252, 255), QColor(255, 206, 90), QColor(120, 230, 255), QColor(214, 250, 255), QColor(0, 229, 255) },
      22, PAT_FAN | PAT_SWEEP | PAT_RAIN, PAT_BURST, BossTrait::FROST, 58, 1.2 },

    // 50: PHOENIX OVERLORD - the final boss
    { "PHOENIX OVERLORD", "The Eternal Egg-Lord",
      { QColor(62, 14, 14),   QColor(229, 57, 53),  QColor(255, 143, 0),  QColor(140, 20, 20),
        QColor(244, 81, 30),  QColor(255, 238, 88), QColor(168, 24, 24),
        QColor(255, 193, 7),  QColor(255, 254, 235), QColor(255, 238, 88), QColor(255, 254, 235), QColor(255, 238, 88) },
      28, PAT_FAN | PAT_SWEEP | PAT_BURST | PAT_RAIN, 0u, BossTrait::INFERNO, 52, 1.3 }
};

const BossDef &bossDefForTier(int tier)
{
    return kBosses[std::clamp(tier, 0, BOSS_TIERS - 1)];
}

// -------------------------------------------------------------
// Procedural sprite: shapes are rasterised into a local canvas
// (y grows downward, x centred) then emitted into the pixel map.
// Every part is stickered with an outline so wings, body and head
// read as separate layers.
// -------------------------------------------------------------
namespace {

constexpr int CW = 51;      // canvas width  (x in [-25, 25])
constexpr int CH = 34;      // canvas height (y 0 = top)
constexpr int CX = 25;

struct Canvas {
    QColor px[CH][CW];
    bool on[CH][CW];

    Canvas() { for (int y = 0; y < CH; ++y) for (int x = 0; x < CW; ++x) on[y][x] = false; }

    bool inb(int x, int y) const { return x >= -CX && x <= CX && y >= 0 && y < CH; }
    void put(int x, int y, const QColor &c) {
        if (!inb(x, y)) return;
        px[y][x + CX] = c;
        on[y][x + CX] = true;
    }
    bool has(int x, int y) const { return inb(x, y) && on[y][x + CX]; }
};

// A part is a mask + colouring function; drawn with an outer outline.
struct Mask {
    bool m[CH][CW];
    Mask() { for (int y = 0; y < CH; ++y) for (int x = 0; x < CW; ++x) m[y][x] = false; }
    void set(int x, int y) { if (x >= -CX && x <= CX && y >= 0 && y < CH) m[y][x + CX] = true; }
    bool get(int x, int y) const { return x >= -CX && x <= CX && y >= 0 && y < CH && m[y][x + CX]; }
};

QColor mix(const QColor &a, const QColor &b, double t)
{
    t = std::clamp(t, 0.0, 1.0);
    return QColor(int(a.red() + (b.red() - a.red()) * t),
                  int(a.green() + (b.green() - a.green()) * t),
                  int(a.blue() + (b.blue() - a.blue()) * t));
}

template <typename F>
void paintPart(Canvas &cv, const Mask &mask, const QColor &outline, F colorFn, bool withOutline = true)
{
    if (withOutline)
    {
        for (int y = -1; y <= CH; ++y)
        {
            for (int x = -CX - 1; x <= CX + 1; ++x)
            {
                if (mask.get(x, y)) continue;
                if (mask.get(x - 1, y) || mask.get(x + 1, y) || mask.get(x, y - 1) || mask.get(x, y + 1))
                {
                    cv.put(x, y, outline);
                }
            }
        }
    }
    for (int y = 0; y < CH; ++y)
    {
        for (int x = -CX; x <= CX; ++x)
        {
            if (mask.get(x, y)) cv.put(x, y, colorFn(x, y));
        }
    }
}

bool inEllipse(double x, double y, double cx, double cy, double rx, double ry)
{
    const double dx = (x - cx) / rx;
    const double dy = (y - cy) / ry;
    return dx * dx + dy * dy <= 1.0;
}

// Light from the upper-left: 0 = shadow ... 1 = highlight
double lightAt(double x, double y, double cx, double cy, double r)
{
    const double v = (-(x - cx) * 0.55 - (y - cy) * 0.85) / r;
    return std::clamp(0.5 + v * 0.5, 0.0, 1.0);
}

void buildWing(Mask &m, int side, int frame)
{
    // Shoulder at x = 6, y = 16; wing runs out to x = 23
    for (int d = 0; d <= 17; ++d)
    {
        double yc;
        if (frame == 0)      yc = 14.0 - d * 0.78;   // wings up
        else if (frame == 1) yc = 14.5 + d * 0.06;   // wings level
        else                 yc = 15.0 + d * 0.72;   // wings down
        const double thick = 10.5 - d * 0.34;
        const double top = yc - thick * 0.38;
        double bot = yc + thick * 0.62;
        if (d > 3)
        {
            static const int teeth[3] = { 0, 1, 2 };
            bot -= teeth[d % 3];                      // scalloped flight feathers
        }
        const int x = side * (6 + d);
        for (int y = int(std::floor(top)); y <= int(std::ceil(bot)); ++y)
        {
            m.set(x, y);
        }
    }
}

} // namespace

void rasterizeBossBird(MyPixels &pixels, const BossBird &boss)
{
    if (!boss.present || !boss.def) return;
    const BossPalette &pal = boss.def->pal;
    Canvas cv;

    const double charge = boss.chargeTicks > 0
        ? 1.0 - static_cast<double>(boss.chargeTicks) / std::max(1, boss.chargeMax) : 0.0;
    const bool dying = boss.defeatTicks > 0;
    const bool flash = boss.hurtFlash > 0 && (boss.hurtFlash % 2 == 1);

    auto tone = [&](const QColor &c) -> QColor {
        QColor r = c;
        if (dying) r = mix(r, QColor(120, 120, 130), 0.55);
        if (flash) r = mix(r, QColor(255, 255, 255), 0.7);
        return r;
    };

    const QColor outline = tone(pal.outline);

    // 1. Wings (behind everything)
    const int wf = boss.wingFrame % 3;
    for (int side = -1; side <= 1; side += 2)
    {
        Mask wm;
        buildWing(wm, side, wf);
        // Wing colouring: bright leading edge, covert band, dark flight feathers with feather splits
        int minY = CH, maxY = 0;
        for (int y = 0; y < CH; ++y) for (int x = -CX; x <= CX; ++x) if (wm.get(x, y)) { minY = std::min(minY, y); maxY = std::max(maxY, y); }
        paintPart(cv, wm, outline, [&](int x, int y) -> QColor {
            const int d = std::abs(x) - 6;
            // find this column's top/bottom
            int colTop = CH, colBot = 0;
            for (int yy = 0; yy < CH; ++yy) if (wm.get(x, yy)) { colTop = std::min(colTop, yy); colBot = std::max(colBot, yy); }
            const double span = std::max(1, colBot - colTop);
            const double f = (y - colTop) / span;
            QColor c;
            if (y - colTop <= 1)        c = pal.wingLight;
            else if (f < 0.42)          c = pal.wing;
            else                        c = pal.wingDark;
            if (f >= 0.42 && d % 3 == 0 && d > 3) c = c.darker(135);       // feather split
            if (f < 0.42 && d % 3 == 1 && f > 0.15) c = mix(c, pal.wingLight, 0.22);
            return tone(c);
        });
    }

    // 2. Feet / talons
    for (int side = -1; side <= 1; side += 2)
    {
        for (int k = 0; k < 3; ++k)
        {
            const int x = side * (3 + k * 2);
            for (int y = 28; y <= 30 + (k == 1 ? 1 : 0); ++y) cv.put(x, y, tone(pal.beak));
            cv.put(x, 31 + (k == 1 ? 1 : 0), tone(mix(pal.beak, pal.outline, 0.5)));
        }
    }

    // 3. Body
    {
        Mask bm;
        for (int y = 0; y < CH; ++y)
            for (int x = -CX; x <= CX; ++x)
                if (inEllipse(x, y, 0, 19.5, 10.5, 9.8)) bm.set(x, y);
        paintPart(cv, bm, outline, [&](int x, int y) -> QColor {
            const double l = lightAt(x, y, 0, 19.5, 12);
            QColor c = l > 0.66 ? pal.bodyLight : (l > 0.30 ? pal.body : pal.bodyDark);
            // chest feather scallops
            if (y > 13 && (y % 3 == 0) && ((x + ((y / 3) % 2) * 2) % 4 == 0)) c = c.darker(118);
            return tone(c);
        });

        // Belly patch
        Mask bl;
        for (int y = 0; y < CH; ++y)
            for (int x = -CX; x <= CX; ++x)
                if (inEllipse(x, y, 0, 21.5, 6.2, 7.0) && bm.get(x, y)) bl.set(x, y);
        paintPart(cv, bl, outline, [&](int x, int y) -> QColor {
            const double l = lightAt(x, y, 0, 21.5, 7);
            QColor c = mix(pal.belly, pal.bodyDark, (1.0 - l) * 0.22);
            if ((y % 3 == 1) && ((x + (y / 3) * 2) % 4 == 0)) c = c.darker(108);
            return tone(c);
        }, false);

        // Egg cannon: glowing pod that charges up before a volley
        const double pulse = 0.5 + 0.5 * std::sin(boss.animTick * 0.35);
        const double glowAmt = std::clamp(0.18 + charge * 0.85 + (charge > 0 ? pulse * 0.15 : 0.0), 0.0, 1.0);
        for (int y = 0; y < CH; ++y)
        {
            for (int x = -CX; x <= CX; ++x)
            {
                const double dx = x, dy = y - 24.5;
                const double dist = std::sqrt(dx * dx + dy * dy);
                if (dist <= 3.1)
                {
                    QColor c = (dist > 2.2) ? mix(pal.outline, pal.glow, 0.35 + glowAmt * 0.3)
                                            : mix(pal.bodyDark, pal.glow, glowAmt);
                    if (dist < 1.2) c = mix(c, QColor(255, 255, 255), glowAmt * 0.6);
                    cv.put(x, y, tone(c));
                }
            }
        }
    }

    // 4. Crown (behind the head)
    {
        struct Spike { int x; int h; };
        const Spike spikes[] = { {0, 6}, {-5, 4}, {5, 4}, {-9, 3}, {9, 3} };
        Mask cm;
        for (const auto &sp : spikes)
        {
            // Triangle pointing up: tip at (sp.x, 7 - sp.h*?), base wider
            const int tipY = 8 - sp.h - (sp.x == 0 ? 2 : 0);
            for (int y = tipY; y <= 9; ++y)
            {
                const int half = (y - tipY) / 2;
                for (int x = sp.x - half; x <= sp.x + half; ++x) cm.set(x, y);
            }
        }
        paintPart(cv, cm, outline, [&](int /*x*/, int y) -> QColor {
            const bool tip = (y <= 5);
            QColor c = tip ? pal.crestLight : pal.crest;
            if (boss.def->trait == BossTrait::GALE && (boss.animTick / 4) % 2 == 0 && tip) c = QColor(255, 255, 255);
            return tone(c);
        });
    }

    // 5. Head
    {
        Mask hm;
        for (int y = 0; y < CH; ++y)
            for (int x = -CX; x <= CX; ++x)
                if (inEllipse(x, y, 0, 12.0, 8.4, 6.2)) hm.set(x, y);
        paintPart(cv, hm, outline, [&](int x, int y) -> QColor {
            const double l = lightAt(x, y, 0, 12.0, 8.5);
            QColor c = l > 0.64 ? pal.bodyLight : (l > 0.28 ? pal.body : pal.bodyDark);
            return tone(c);
        });

        // Cheek patches
        for (int side = -1; side <= 1; side += 2)
        {
            cv.put(side * 6, 14, tone(pal.belly));
            cv.put(side * 5, 15, tone(pal.belly));
            cv.put(side * 6, 15, tone(pal.belly));
        }

        // Angry brow ridges (inner ends lower) + glaring eyes
        const bool rage = boss.enraged;
        const QColor eyeWhite = tone(rage ? QColor(255, 235, 150) : QColor(255, 255, 255));
        const QColor pupil = tone(rage ? QColor(220, 20, 40) : QColor(18, 22, 40));
        for (int side = -1; side <= 1; side += 2)
        {
            for (int k = 0; k < 6; ++k)
            {
                const int x = side * (1 + k);
                const int y = 8 + (5 - k) / 3 + (k < 1 ? 1 : 0);
                cv.put(x, y, tone(pal.outline));
                cv.put(x, y + 1, tone(pal.outline));
            }
            // eye 3x3 under brow
            for (int ey = 0; ey < 3; ++ey)
                for (int ex = 0; ex < 3; ++ex)
                    cv.put(side * (3 + ex), 11 + ey, eyeWhite);
            if (dying)
            {
                // X eyes
                cv.put(side * 3, 11, outline); cv.put(side * 5, 11, outline);
                cv.put(side * 4, 12, outline);
                cv.put(side * 3, 13, outline); cv.put(side * 5, 13, outline);
            }
            else
            {
                // pupil looks toward the centre for a menacing glare
                cv.put(side * 3, 12, pupil);
                cv.put(side * 3, 13, pupil);
                cv.put(side * 4, 12, pupil);
            }
        }

        // Hooked beak
        Mask bk;
        for (int y = 14; y <= 20; ++y)
        {
            const int half = (y <= 15) ? 2 : (y <= 18 ? 1 : 0);
            for (int x = -half; x <= half; ++x) bk.set(x, y);
        }
        bk.set(0, 21);
        paintPart(cv, bk, outline, [&](int x, int y) -> QColor {
            QColor c = pal.beak;
            if (y >= 18) c = mix(pal.beak, pal.outline, 0.35);
            if (y == 14 && std::abs(x) <= 1) c = mix(pal.beak, QColor(255, 255, 255), 0.35);
            return tone(c);
        });
    }

    // Emit to the pixel map (anchor: canvas row 17 sits on boss.mathY)
    const int bx = static_cast<int>(std::round(boss.mathX));
    const int by = static_cast<int>(std::round(boss.mathY));
    for (int y = 0; y < CH; ++y)
    {
        for (int x = -CX; x <= CX; ++x)
        {
            if (cv.on[y][x + CX])
            {
                pixels.setPixel(bx + x, by + (17 - y), cv.px[y][x + CX]);
            }
        }
    }
}
