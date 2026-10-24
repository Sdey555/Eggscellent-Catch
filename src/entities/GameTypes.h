#ifndef GAMETYPES_H
#define GAMETYPES_H

enum class EggType {
    REGULAR,         // Standard egg (+10 pts, miss = -1 heart)
    GOLDEN,          // Special bonus egg (+50 pts, miss = -1 heart)
    BIRD_POOP,       // Bird poop (catch = -1 heart, miss = safe!)
    BASKET_GROW,     // Rare growth egg (increases basket width +2, bonus +25 pts)
    BASKET_RESTORE   // Very rare legendary egg (restores basket to starting width 9, bonus +100 pts)
};

inline constexpr EggType BOMB = EggType::BIRD_POOP;

enum class GameMode {
    EASY,
    MEDIUM,
    HARD
};

struct StickyPuddle {
    double mathX{0.0};
    double radius{3.5};
    int lifeTicks{350};
    int maxLifeTicks{350};
};

struct CompanionBasket {
    bool active{false};
    double mathX{0.0};
    double mathY{0.0};
    int halfWidth{6};
    int height{7};
    double speed{4.5};
    int ticksLeft{0};
    int maxTicks{165}; // ~5.0 seconds at 30ms/tick
};

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class SoundId {
    CATCH_REGULAR,       // Standard white egg caught
    CATCH_GOLDEN,        // Golden egg caught
    CATCH_GROW,          // Green basket growth egg caught
    CATCH_LEGENDARY,     // Mythical Phoenix egg caught
    SPLAT,               // Egg dropped & splattered on ground
    POOP_HIT,            // Bird poop hit basket (heart lost)
    POOP_GROUND,         // Bird poop hit ground (sticky puddle formed)
    THUNDER,             // Thunder strike / lightning bolt impact
    SHOCK,               // Basket shocked by lightning
    LEVEL_UP,            // Level up victory fanfare
    GAME_OVER,           // Player loses all hearts (Game Over)
    GROW_USE,            // 'H' key used to expand basket
    COMPANION_SPAWN,     // 'C' key used to summon companion basket
    UI_CLICK             // Button click, mode toggle, pause toggle
};

#endif // GAMETYPES_H
