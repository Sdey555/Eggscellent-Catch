#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "GameTypes.h"
#include "Basket.h"
#include "Bird.h"
#include "FallingEgg.h"
#include "Particle.h"
#include <vector>
#include <random>
#include <QString>

class MyGrid;

// -------------------------------------------------------------
// Core Game Engine: Manages Rules, Entities, Physics, Difficulty
// -------------------------------------------------------------
class GameEngine
{
public:
    static constexpr int MAX_LEVEL = 50;
    static constexpr int POINTS_PER_LEVEL = 100;

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

    const Basket& getBasket() const { return basket; }
    const std::vector<Bird>& getBirds() const { return birds; }
    const FallingEgg& getFallingEgg() const { return fallingEgg; }
    bool isLegendaryBirdActive() const { return legendaryBirdActive; }
    const Bird& getLegendaryBird() const { return legendaryBird; }
    const std::vector<GameParticle>& getParticles() const { return particles; }
    const std::vector<FloatingText>& getFloatingTexts() const { return floatingTexts; }

    int levelForScore(int s) const;
    double levelSpeedMultiplier() const;

private:
    GameState gameState{GameState::MENU};
    int menuSlideIndex{0};
    int score{0};
    int highScore{0};
    int hearts{3};
    int level{1};
    int levelBannerTicks{0};
    int lastRestoreEggLevel{0};
    int lastGrowEggMissLevel{0};
    QString gameOverReason;
    QString statusMessage;

    Basket basket;
    std::vector<Bird> birds;
    FallingEgg fallingEgg;
    std::vector<GameParticle> particles;
    std::vector<FloatingText> floatingTexts;

    int eggSpawnCooldown{100};
    int groundMathY{-24};

    bool keyLeftPressed{false};
    bool keyRightPressed{false};
    bool mouseControl{false};
    int mouseScreenX{0};
    int lastMouseX{0};

    std::mt19937 rng;

    // Special Legendary Bird (spawns exclusively to lay the Legendary Egg)
    bool legendaryBirdActive{false};
    Bird legendaryBird;
    int legendaryPathType{0}; // 0 = SINE_WAVE, 1 = SWOOP_ARC, 2 = DIAGONAL_GLIDE, 3 = HORIZONTAL_SURGE
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

    Bird createBird(size_t index, const MyGrid &grid);
    void updatePhysics(const MyGrid &grid);
    void updateBirds(const MyGrid &grid);
    void updateLegendaryBird(const MyGrid &grid);
    void spawnLegendaryBird(const MyGrid &grid);
    void updateEggSpawn(const MyGrid &grid);
    void updateFallingEgg(const MyGrid &grid);
    void updateParticles();
    void checkLevelUp(const MyGrid &grid);
    void applyLevelSettings(const MyGrid &grid);

    void spawnCatchParticles(double x, double y, const QColor &col, int count = 14);
    void spawnSplatParticles(double x, double y, EggType type);
    void spawnPoopSplashParticles(double x, double y);
    void spawnExplosionParticles(double x, double y);
    void addFloatingText(double x, double y, const QString &text, const QColor &col);
};

#endif // GAMEENGINE_H
