#ifndef GAMETYPES_H
#define GAMETYPES_H

enum class EggType {
    REGULAR,    // Standard egg (+10 pts, miss = -1 heart)
    GOLDEN,     // Special bonus egg (+50 pts, miss = -1 heart)
    BOMB        // False egg (bomb: catch = instant game over, miss = safe!)
};

enum class GameState {
    PLAYING,
    PAUSED,
    GAME_OVER
};

#endif // GAMETYPES_H
