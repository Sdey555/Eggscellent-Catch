#ifndef BOSS_H
#define BOSS_H

#include <QColor>
#include "GameTypes.h"

class MyPixels;

// -------------------------------------------------------------
// Boss birds: one appears at level 3, then every 10 levels (3, 10, 20, 30, 40, 50).
// Each boss has its own palette, volley patterns and a special
// trait that links into the weather / ground-hazard systems.
// -------------------------------------------------------------

enum BossPattern : unsigned {
    PAT_FAN   = 1u << 0,  // 5-lane fan: one real egg hidden among droppings
    PAT_SWEEP = 1u << 1,  // boss strafes edge to edge dropping a curtain
    PAT_BURST = 1u << 2,  // two quick fans back-to-back
    PAT_RAIN  = 1u << 3   // random downpour with a few eggs mixed in (rage only for most bosses)
};

enum class BossTrait {
    NONE,     // Plain
    GALE,     // Rage summons howling crosswinds
    MUD,      // Its droppings leave heavy mud
    FROST,    // Its droppings leave ice
    INFERNO   // Final boss: mud + gale in rage
};

struct BossPalette {
    QColor outline, body, bodyLight, bodyDark;
    QColor wing, wingLight, wingDark;
    QColor belly, beak, crest, crestLight, glow;
};

struct BossDef {
    const char *name;
    const char *title;
    BossPalette pal;
    int baseHp;             // HP at Medium difficulty
    unsigned patterns;      // patterns used before rage
    unsigned rageExtra;     // patterns added when enraged
    BossTrait trait;
    int volleyGap;          // ticks between volleys (shorter = harder)
    double travel;          // movement speed multiplier
};

struct BossBird {
    bool present{false};
    int tier{0};
    double mathX{0.0};
    double mathY{0.0};
    int hp{1};
    int maxHp{1};
    int wingFrame{0};
    int wingTick{0};
    int animTick{0};
    int chargeTicks{0};     // >0 while telegraphing a volley
    int chargeMax{1};
    int hurtFlash{0};
    bool enraged{false};
    int defeatTicks{0};     // >0 while the boss is being defeated
    const BossDef *def{nullptr};
};

constexpr int BOSS_TIERS = 5;
constexpr int BOSS_HALF_HEIGHT = 17;   // rows above the anchor the sprite may occupy (16 below)
constexpr int BOSS_BELLY_DROP = 10;    // eggs leave the boss this many rows below its anchor

const BossDef &bossDefForTier(int tier);
inline int bossTierForLevel(int level) {
    const int t = (level / 10) - 1;   // 10->0, 20->1, 30->2, 40->3, 50->4 (climax at max level)
    return t < 0 ? 0 : (t >= BOSS_TIERS ? BOSS_TIERS - 1 : t);
}

void rasterizeBossBird(MyPixels &pixels, const BossBird &boss);

#endif // BOSS_H
