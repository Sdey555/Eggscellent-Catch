#ifndef GAMETYPES_H
#define GAMETYPES_H

enum class EggType {
    REGULAR,         // Standard egg (+10 pts, miss = -1 heart)
    GOLDEN,          // Special bonus egg (+50 pts, miss = -1 heart)
    BIRD_POOP,       // Bird poop (catch = -1 heart, miss = safe!)
    BASKET_GROW,     // Rare growth egg (increases basket width +2, bonus +25 pts)
    BASKET_RESTORE,  // Very rare legendary egg (restores basket to starting width 9, bonus +100 pts)
    BOSS_REWARD      // Trophy egg dropped by a defeated boss (+100 pts, heart/growth eggs, wider basket; safe to miss)
};

inline constexpr EggType BOMB = EggType::BIRD_POOP;

enum class GameMode {
    EASY,
    MEDIUM,
    HARD
};

// Ground hazard patch families. SLIME is the original sticky poop puddle.
enum class PatchType {
    SLIME,   // Sticky poop puddle: basket moves at 45% speed
    MUD,     // Rain-soaked mud: heavy slow (30% speed), forms where the ground is wet
    ICE      // Night frost: normal speed but the basket slides and is hard to stop
};

struct StickyPuddle {
    double mathX{0.0};
    double radius{3.5};
    int lifeTicks{350};
    int maxLifeTicks{350};
    PatchType type{PatchType::SLIME};
    int ageTicks{0};

    static constexpr int FORM_TICKS = 22; // Mud / ice visibly form before they bite
    static constexpr int MELT_TICKS = 45; // ...and shrink away at the end of their life

    // Effective radius used by BOTH physics and rendering so what you see is what you get.
    // Slime keeps its original constant radius; mud and ice grow in and shrink out.
    double currentRadius() const {
        if (type == PatchType::SLIME) {
            return radius;
        }
        const double grow = ageTicks >= FORM_TICKS ? 1.0 : static_cast<double>(ageTicks) / FORM_TICKS;
        const double fade = lifeTicks >= MELT_TICKS ? 1.0 : static_cast<double>(lifeTicks) / MELT_TICKS;
        const double f = grow < fade ? grow : fade;
        return radius * f;
    }
    bool isForming() const { return type != PatchType::SLIME && ageTicks < FORM_TICKS; }
};

// Gust life-cycle: calm -> telegraphed warning -> blowing
enum class WindPhase {
    CALM,
    WARNING,
    GUST
};

// Boss fight life-cycle (a boss appears every 10 levels)
enum class BossPhase {
    NONE,      // No boss: normal play
    PENDING,   // Boss level reached; waiting for the sky to clear before the intro
    INTRO,     // Warning banner, boss descends
    FIGHT,     // Active fight
    DEFEATED   // Boss beaten: victory burst, trophy egg, boss flies away
};

// Grand drifting pixel clouds shared between physics and rendering
struct CloudBankDef {
    int speedDivider;
    int tickOffset;
    int relY;
    int width;
    int height;
};

inline constexpr int NUM_CLOUD_BANKS = 4;

inline const CloudBankDef& getCloudDef(int index) {
    static const CloudBankDef clouds[NUM_CLOUD_BANKS] = {
        { 5,  15, -16, 48, 14 },
        { 7, 140, -26, 40, 12 },
        { 4, 270, -38, 54, 15 },
        { 6, 390, -20, 36, 11 }
    };
    return clouds[(index >= 0 ? index : -index) % NUM_CLOUD_BANKS];
}

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
    SETTINGS,
    GAME_OVER
};

enum class ControlMode {
    SCREEN_TOUCH,        // Touch or drag anywhere on screen (speed-limited)
    VIRTUAL_BUTTONS,     // On-screen Left/Right touch buttons for thumbs
    GYROSCOPE,           // Tilt phone left/right to move basket
    MOUSE_KEYBOARD       // Mouse cursor tracking + keyboard A/D or arrows
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
    UI_CLICK,            // Button click, mode toggle, pause toggle
    BOSS_WARNING,        // Boss incoming siren
    BOSS_HIT,            // Boss takes damage from a caught egg
    BOSS_DEFEAT,         // Boss defeated fanfare
    WIND_GUST,           // A gust of wind begins
    FROST_FORM           // Ice patch crystallises on the ground
};

#endif // GAMETYPES_H
