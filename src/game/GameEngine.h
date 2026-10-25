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
    static constexpr int POINTS_PER_LEVEL = 100;
    static constexpr int DAY_NIGHT_CYCLE_TICKS = 2400; // ~72s per full day-night cycle

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

    // Audio Callback
    using SoundCallback = std::function<void(SoundId)>;
    void setSoundCallback(SoundCallback cb) { soundCallback = cb; }

    // Accessors
    GameState getState() const { return gameState; }
    void setState(GameState s) { gameState = s; }
    int getMenuSlideIndex() const { return menuSlideIndex; }
    void setMenuSlideIndex(int idx) { menuSlideIndex = idx; }
    int getScore() const { return score; }
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
    void addWarningFloatingText(double x, double y, const QString &text, const QColor &col);

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
    bool isBasketInStickyPuddle() const { return basketInStickyPuddle; }
    bool isBirdInCloud(const Bird &bird, const MyGrid &grid) const;
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

    // Difficulty helpers
    int levelForScore(int s) const;
    double levelSpeedMultiplier() const;
    double levelPoopProbability() const;
    double levelEggFallSpeed() const;

private:
    GameState gameState{GameState::MENU};
    GameMode mode{GameMode::EASY};
    int menuSlideIndex{0};

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
    int cloudDropCooldownTicks{600};
    bool basketInStickyPuddle{false};
    bool gameAssistEnabled{true};
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

    bool keyLeftPressed{false};
    bool keyRightPressed{false};
    bool mouseControl{false};
    bool mouseControlAllowed{true};
    bool lightningFlashEnabled{true};
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

    void spawnEggFromBird(Bird &bird, const MyGrid &grid, EggType eType);
    void spawnCatchParticles(double x, double y, const QColor &col, int count = 14);
    void spawnSplatParticles(double x, double y, EggType type);
    void spawnPoopSplashParticles(double x, double y);
    void spawnExplosionParticles(double x, double y);
    void addFloatingText(double x, double y, const QString &text, const QColor &col);
};

#endif // GAMEENGINE_H
