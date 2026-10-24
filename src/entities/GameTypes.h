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

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

#endif // GAMETYPES_H
