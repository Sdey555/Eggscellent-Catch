#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <vector>
#include <random>
#include <functional>
#include <QString>
#include <QColor>
#include "GameTypes.h"
#include "Basket.h"
#include "Bird.h"
#include "Boss.h"
#include "FallingEgg.h"
#include "Particle.h"
#include "Grid.h"
#include "MotionSensor.h"

// -------------------------------------------------------------
// Core Game Engine: Manages Rules, Entities, Physics, Difficulty
// -------------------------------------------------------------
class GameEngine
{
public:
    static constexpr int MAX_LEVEL = 50;
    static constexpr int POINTS_PER_LEVEL = 100;       // standard level length (level 3 onwards)
    static constexpr int LEVEL1_POINTS = 50;           // levels 1 and 2 are shorter
    static constexpr int LEVEL2_POINTS = 70;
    static constexpr int levelLength(int lvl) { return lvl <= 1 ? LEVEL1_POINTS : (lvl == 2 ? LEVEL2_POINTS : POINTS_PER_LEVEL); }
    static constexpr int levelStartScore(int lvl)
    {
        return lvl <= 1 ? 0 : (lvl == 2 ? LEVEL1_POINTS : LEVEL1_POINTS + LEVEL2_POINTS + (lvl - 3) * POINTS_PER_LEVEL);
    }
    static constexpr int DAY_NIGHT_CYCLE_TICKS = 2400; // ~72s per full day-night cycle
    static constexpr int BOSS_BANNER_TICKS = 110;       // boss warning banner length
    static constexpr double WIND_DRIFT = 0.20;          // egg drift per tick at full wind

    GameEngine();

    void init(const MyGrid &grid);
    void reset(const MyGrid &grid);
    void togglePause();
    void tick(const MyGrid &grid);

    // Input handlers
    void setKeyLeft(bool pressed);
    void setKeyRight(bool pressed);
    void handleMouseMove(int screenX);
    void handleMouseLeftClick(int clickX);
    void handleMouseRightClick();

    // Mode selection & special action keys
    GameMode getMode() const { return mode; }
    void setMode(GameMode m);
    void setMode(GameMode m, const MyGrid &grid);
    void cycleMode(const MyGrid &grid);
    void useGrowthEgg();           // Triggered by 'H' key
    void summonCompanionBasket();   // Triggered by 'C' key (Hard mode)
    void defend();                  // Defend basket against swooping bird attack ([SPACE] / Tap)

    // Audio Callback
    using SoundCallback = std::function<void(SoundId)>;
    void setSoundCallback(SoundCallback cb) { soundCallback = cb; }

    // Accessors
    GameState getState() const { return gameState; }
    void setState(GameState s) { gameState = s; }
    int getMenuSlideIndex() const { return menuSlideIndex; }
    void setMenuSlideIndex(int idx) { menuSlideIndex = idx; }
    int getMenuScrollY() const { return menuScrollY; }
    void setMenuScrollY(int y) { menuScrollY = std::max(0, y); }
    void scrollMenu(int dy, int maxScroll) { menuScrollY = std::clamp(menuScrollY + dy, 0, maxScroll); }

    int getPauseScrollY() const { return pauseScrollY; }
    void setPauseScrollY(int y) { pauseScrollY = std::max(0, y); }
    void scrollPause(int dy, int maxScroll) { pauseScrollY = std::clamp(pauseScrollY + dy, 0, maxScroll); }

    int getSettingsScrollY() const { return settingsScrollY; }
    void setSettingsScrollY(int y) { settingsScrollY = std::max(0, y); }
    void scrollSettings(int dy, int maxScroll) { settingsScrollY = std::clamp(settingsScrollY + dy, 0, maxScroll); }
    int getScore() const { return score; }
    // Score that counts toward level progress (boss-fight points are banked as score only,
    // so beating a boss lands you cleanly on the next level rather than skipping several)
    int getProgressScore() const { return score - levelCredit; }
    int getHighScore() const { return highScore; }
    int getHearts() const { return hearts; }
    int getLevel() const { return level; }
    int getLevelBannerTicks() const { return levelBannerTicks; }
    int getGroundMathY() const { return groundMathY; }
    const QString& getGameOverReason() const { return gameOverReason; }
    const QString& getStatusMessage() const { return statusMessage; }
    void setStatusMessage(const QString &msg) { statusMessage = msg; }

    bool isMouseControl() const { return mouseControl; }
    bool isMouseControlAllowed() const { return mouseControlAllowed; }
    void setMouseControlAllowed(bool allowed) { mouseControlAllowed = allowed; }
    bool isLightningFlashEnabled() const { return lightningFlashEnabled; }
    void setLightningFlashEnabled(bool enabled) { lightningFlashEnabled = enabled; }
    bool isGameAssistEnabled() const { return gameAssistEnabled; }
    void setGameAssistEnabled(bool enabled) { gameAssistEnabled = enabled; }
    void moveBasketBy(double delta) { basket.mathX += delta; }

    ControlMode getControlMode() const { return controlMode; }
    void setControlMode(ControlMode m);
    float getGyroTilt() const { return gyroTilt; }
    void setGyroTilt(float t) { gyroTilt = t; }
    bool isKeyLeftPressed() const { return keyLeftPressed; }
    bool isKeyRightPressed() const { return keyRightPressed; }
    bool isTouchActive() const { return touchActive; }
    void handleTouchRelease() { touchActive = false; }

    int getGrowthEggCount() const { return growthEggCount; }
    const Basket& getBasket() const { return basket; }
    const std::vector<Bird>& getBirds() const { return birds; }
    const std::vector<FallingEgg>& getFallingEggs() const { return fallingEggs; }
    const FallingEgg& getFallingEgg() const; // Backwards compatibility
    const CompanionBasket& getCompanionBasket() const { return companionBasket; }
    const std::vector<StickyPuddle>& getStickyPuddles() const { return stickyPuddles; }
    bool isLegendaryBirdActive() const { return legendaryBirdActive; }
    const Bird& getLegendaryBird() const { return legendaryBird; }
    const std::vector<GameParticle>& getParticles() const { return particles; }
    const std::vector<FloatingText>& getFloatingTexts() const { return floatingTexts; }

    // Day/Night & Weather Accessors
    int getDayNightTick() const { return dayNightTick; }
    bool isThunderstormActive() const { return thunderstormActive; }
    int getThunderstormTicks() const { return thunderstormTicks; }
    int getLightningFlashTicks() const { return lightningFlashTicks; }
    double getDayNightProgress() const;
    double getNightDurationRatio() const;

    // Thunderstrike & Shock Accessors
    int getLightningShockTimer() const { return lightningShockTimer; }
    int getLightningStrikeState() const { return lightningStrikeState; }
    int getLightningStrikeTimer() const { return lightningStrikeTimer; }
    double getLightningStrikeX() const { return lightningStrikeX; }

    // Bird Attack & Basket Defense
    bool isBasketDefending() const { return basketDefending; }
    int getBasketDefendCooldown() const { return basketDefendCooldown; }

    // ---------------------------------------------------------
    // Boss fights (every 10 levels: 10, 20, 30, 40, 50)
    // ---------------------------------------------------------
    static bool isBossLevel(int lvl) { return (lvl >= 10 && lvl % 10 == 0 && lvl <= MAX_LEVEL); }
    BossPhase getBossPhase() const { return bossPhase; }
    bool isBossActive() const { return bossPhase != BossPhase::NONE; }
    const BossBird& getBoss() const { return boss; }
    int getBossBannerTicks() const { return bossBannerTicks; }
    int getBossPhaseTicks() const { return bossPhaseTicks; }
    int getBossBannerMax() const { return BOSS_BANNER_TICKS; }
    int getBossPendingLevel() const { return level; }
    int getScreenShakeTicks() const { return screenShakeTicks; }
    int getScreenShakeMag() const { return screenShakeMag; }
    int getBossesDefeated() const { return bossesDefeated; }

    // ---------------------------------------------------------
    // Gameplay weather
    // ---------------------------------------------------------
    // Wind: signed, -1 (blowing left) .. +1 (blowing right); eggs drift by wind * WIND_DRIFT per tick
    double getWind() const { return windStrength; }
    WindPhase getWindPhase() const { return windPhase; }
    int getWindWarningTicks() const { return windPhase == WindPhase::WARNING ? windPhaseTicks : 0; }
    double getWindIncomingDirection() const { return windIncomingDir; }
    bool isRainShowerActive() const { return rainActive; }
    double getWetness() const { return wetness; }       // 0 = dry, 1 = soaked (basket slides)
    double getFogDensity() const { return fogDensity; } // 0 = clear night, ~0.8 = thick
    double getNightFactor() const;                        // 0 day .. 1 deep night (smooth)
    bool isBasketSliding() const { return basketSlipAlpha < 0.8; }

    // Ground hazards under the basket
    bool isBasketOnPatch() const { return basketOnPatch; }
    PatchType getBasketPatchType() const { return basketPatchType; }

    // Difficulty helpers
    int levelForScore(int s) const;
    double levelSpeedMultiplier() const;
    double levelPoopProbability() const;
    double levelEggFallSpeed() const;

private:
    GameState gameState{GameState::MENU};
    GameMode mode{GameMode::EASY};
    int menuSlideIndex{0};
    int menuScrollY{0};
    int pauseScrollY{0};
    int settingsScrollY{0};

    int score{0};
    int highScore{0};
    int hearts{3};
    int level{1};
    int levelBannerTicks{0};
    int lastRestoreEggLevel{0};
    int lastGrowEggMissLevel{0};
    int growthEggCount{0};
    QString gameOverReason;
    QString statusMessage;

    Basket basket;
    CompanionBasket companionBasket;
    std::vector<Bird> birds;
    std::vector<FallingEgg> fallingEggs;
    std::vector<StickyPuddle> stickyPuddles;
    std::vector<GameParticle> particles;
    std::vector<FloatingText> floatingTexts;

    int eggSpawnCooldown{100};
    int dualDropCooldownTicks{900};
    int groundMathY{-24};

    // Day / Night & Weather
    int dayNightTick{0};
    bool thunderstormActive{false};
    int thunderstormTicks{0};
    int thunderstormCooldown{3000}; // Rare: 90-150s cooldown
    int lightningFlashTicks{0};

    // Thunderstrike & Basket Shock
    int lightningShockTimer{0};
    int lightningStrikeState{0}; // 0 = idle, 1 = warning sizzle, 2 = striking
    int lightningStrikeTimer{0};
    double lightningStrikeX{0.0};

    // Bird Attack & Basket Defense (From Level 1 onwards)
    int birdAttackCooldownTicks{280};
    bool basketDefending{false};
    int basketDefendTicks{0};
    int basketDefendCooldown{0};

    bool keyLeftPressed{false};
    bool keyRightPressed{false};
    bool mouseControl{false};
    bool mouseControlAllowed{true};
    bool lightningFlashEnabled{true};
    bool gameAssistEnabled{true};
#if defined(Q_OS_ANDROID)
    ControlMode controlMode{ControlMode::SCREEN_TOUCH};
#else
    ControlMode controlMode{ControlMode::MOUSE_KEYBOARD};
#endif
    float gyroTilt{0.0f};
    bool touchActive{false};
    int mouseScreenX{0};
    int lastMouseX{0};

    std::mt19937 rng;

    // Special Legendary Bird (spawns exclusively to lay the Legendary Egg)
    bool legendaryBirdActive{false};
    Bird legendaryBird;
    int legendaryPathType{0};
    double legendaryStartX{0.0};
    double legendaryStartY{0.0};
    double legendaryBaseY{0.0};
    double legendaryAmplitude{0.0};
    double legendaryFrequency{0.0};
    double legendarySpeedX{0.0};
    double legendarySpeedY{0.0};
    double legendaryDropX{0.0};
    bool legendaryEggDropped{false};
    int legendaryAnimTick{0};

    // ---------------------------------------------------------
    // Boss state
    // ---------------------------------------------------------
    struct BossShot {
        int delay{0};            // ticks until this egg leaves the boss
        double offsetX{0.0};     // launch offset from the boss centre
        double landX{0.0};       // where the egg should reach the ground
        bool straight{true};     // true = drop straight down, ignore landX
        EggType type{EggType::BIRD_POOP};
    };
    BossPhase bossPhase{BossPhase::NONE};
    BossBird boss;
    int bossPhaseTicks{0};
    int bossBannerTicks{0};
    int bossNextVolley{60};
    unsigned bossPattern{0};
    unsigned bossLastPattern{0};
    double bossTargetX{0.0};
    double bossHoverY{0.0};
    int bossSweepTicks{0};
    double bossSweepVx{0.0};
    std::vector<BossShot> bossShots;
    bool bossRewardDropped{false};
    bool bossGale{false};
    int highestBossBeaten{0};
    int levelCredit{0};
    int bossesDefeated{0};
    int screenShakeTicks{0};
    int screenShakeMag{0};

    // ---------------------------------------------------------
    // Weather state
    // ---------------------------------------------------------
    double windStrength{0.0};
    double windTarget{0.0};
    double windIncomingDir{1.0};
    double windGustStrength{0.0};
    WindPhase windPhase{WindPhase::CALM};
    int windPhaseTicks{600};
    bool windForced{false};
    bool rainActive{false};
    int rainTicks{0};
    int rainCooldown{1400};
    double wetness{0.0};
    double fogDensity{0.0};
    bool fogAnnounced{false};
    double basketSlideVel{0.0};
    double basketSlipAlpha{1.0};

    // Ground hazards
    int mudCooldown{300};
    int frostCooldown{200};
    bool basketOnPatch{false};
    PatchType basketPatchType{PatchType::SLIME};

    struct PatchEffect {
        double speedMul{1.0};
        double slipAlpha{1.0};
    };

    SoundCallback soundCallback{nullptr};
    void triggerSound(SoundId id) {
        if (soundCallback) {
            soundCallback(id);
        }
    }

    Bird createBird(size_t index, const MyGrid &grid);
    void updatePhysics(const MyGrid &grid);
    void updateBirds(const MyGrid &grid);
    void updateLegendaryBird(const MyGrid &grid);
    void spawnLegendaryBird(const MyGrid &grid);
    void updateEggSpawn(const MyGrid &grid);
    void updateFallingEggs(const MyGrid &grid);
    void updateWeather();
    void updateThunderstormStrike(const MyGrid &grid);
    void updateCompanionBasket(const MyGrid &grid);
    void updateParticles();
    void checkLevelUp(const MyGrid &grid);
    void applyLevelSettings(const MyGrid &grid);

    // Boss
    void updateBoss(const MyGrid &grid);
    void startBossIntro(const MyGrid &grid);
    void updateBossFight(const MyGrid &grid);
    void updateBossDefeat(const MyGrid &grid);
    void beginBossCharge(const MyGrid &grid);
    void fireBossVolley(const MyGrid &grid);
    void spawnBossEgg(const BossShot &shot, const MyGrid &grid);
    void enterBossRage();
    void damageBoss(int dmg, double x, double y);
    void defeatBoss();
    double bossEggSpeedMul() const;
    int estimateFallTicks(double startY, double speedMul) const;
    void shake(int ticks, int mag);

    // Weather & hazards
    void updateWeatherEffects(const MyGrid &grid);
    void updateWind();
    void updateRainAndFog();
    PatchEffect updatePatches(const MyGrid &grid);
    void updateHazardSpawns(const MyGrid &grid);
    void spawnPatch(PatchType type, double x, double radius, int life);

    void spawnEggFromBird(Bird &bird, const MyGrid &grid, EggType eType);
    void spawnCloudPuffParticles(double x, double y, int count = 10);
    void spawnCatchParticles(double x, double y, const QColor &col, int count = 14);
    void spawnSplatParticles(double x, double y, EggType type);
    void spawnPoopSplashParticles(double x, double y);
    void spawnExplosionParticles(double x, double y);
    void addFloatingText(double x, double y, const QString &text, const QColor &col);
};

#endif // GAMEENGINE_H
