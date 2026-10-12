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

    void updatePhysics(const MyGrid &grid);
    void updateBirds(const MyGrid &grid);
    void updateEggSpawn(const MyGrid &grid);
    void updateFallingEgg();
    void updateParticles();
    void checkLevelUp();
    void applyLevelSettings();

    void spawnCatchParticles(double x, double y, const QColor &col, int count = 14);
    void spawnSplatParticles(double x, double y, EggType type);
    void spawnExplosionParticles(double x, double y);
    void addFloatingText(double x, double y, const QString &text, const QColor &col);
};

#endif // GAMEENGINE_H
