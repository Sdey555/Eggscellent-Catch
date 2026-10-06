#include "GameEngine.h"
#include "Grid.h"
#include <algorithm>
#include <cmath>

GameEngine::GameEngine()
{
    rng.seed(std::random_device{}());
}

void GameEngine::init(const MyGrid &grid)
{
    score = 0;
    hearts = 3;
    level = 1;
    levelBannerTicks = 0;
    gameState = GameState::PLAYING;
    gameOverReason.clear();

    keyLeftPressed = false;
    keyRightPressed = false;
    mouseControl = false;

    fallingEgg.active = false;
    fallingEgg.animTick = 0;
    fallingEgg.vy = 0.08;
    fallingEgg.gravity = 0.015;
    fallingEgg.maxSpeed = 1.6;

    particles.clear();
    floatingTexts.clear();

    const int width = std::max(grid.getWidth(), 640);
    const int height = std::max(grid.getHeight(), 480);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);
    const int minMathY = grid.screenToMathY(height);
    const int maxMathY = grid.screenToMathY(0);

    groundMathY = minMathY + 9;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    // Initialize Basket
    basket.mathX = (minMathX + maxMathX) / 2.0;
    basket.mathY = groundMathY + 1;
    basket.halfWidth = 6;
    basket.height = 5;
    basket.speed = std::clamp(screenSpan / 70.0, 1.6, 5.2);

    applyLevelSettings();

    // Initialize Birds at high altitude
    birds.clear();

    Bird b1;
    b1.mathX = minMathX + 18;
    b1.mathY = maxMathY - 9;
    b1.speed = std::clamp(screenSpan / 340.0, 0.40, 1.3);
    b1.direction = 1;
    b1.bodyColor = QColor(52, 152, 219);  // Bluebird
    b1.wingColor = QColor(27, 79, 114);
    b1.bellyColor = QColor(243, 156, 18);
    birds.push_back(b1);

    Bird b2;
    b2.mathX = maxMathX - 22;
    b2.mathY = maxMathY - 15;
    b2.speed = std::clamp(screenSpan / 420.0, 0.32, 1.1);
    b2.direction = -1;
    b2.bodyColor = QColor(231, 76, 60);   // Robin
    b2.wingColor = QColor(120, 40, 31);
    b2.bellyColor = QColor(250, 215, 160);
    birds.push_back(b2);

    Bird b3;
    b3.mathX = (minMathX + maxMathX) / 2.0;
    b3.mathY = maxMathY - 12;
    b3.speed = std::clamp(screenSpan / 280.0, 0.48, 1.5);
    b3.direction = 1;
    b3.bodyColor = QColor(46, 204, 113);  // Greenfinch
    b3.wingColor = QColor(20, 90, 50);
    b3.bellyColor = QColor(249, 231, 159);
    birds.push_back(b3);

    eggSpawnCooldown = 90;
    statusMessage = "Game Started! Move basket with [A/D], [◄/►], or Mouse!";
}

void GameEngine::reset(const MyGrid &grid)
{
    init(grid);
}

void GameEngine::togglePause()
{
    if (gameState == GameState::GAME_OVER)
    {
        // Calling code should trigger reset
        return;
    }
    else if (gameState == GameState::PLAYING)
    {
        gameState = GameState::PAUSED;
        statusMessage = "Game Paused. Press Space / Right-Click / Left-Click to Resume.";
    }
    else if (gameState == GameState::PAUSED)
    {
        gameState = GameState::PLAYING;
        statusMessage = "Game Resumed!";
    }
}

void GameEngine::tick(const MyGrid &grid)
{
    if (gameState == GameState::PLAYING)
    {
        updatePhysics(grid);
    }
}

void GameEngine::setKeyLeft(bool pressed)
{
    keyLeftPressed = pressed;
    if (pressed)
    {
        mouseControl = false;
    }
}

void GameEngine::setKeyRight(bool pressed)
{
    keyRightPressed = pressed;
    if (pressed)
    {
        mouseControl = false;
    }
}

void GameEngine::handleMouseMove(int screenX)
{
    const int dx = std::abs(screenX - lastMouseX);
    lastMouseX = screenX;
    if (mouseControl || dx >= 3)
    {
        mouseScreenX = screenX;
        mouseControl = true;
    }
}

void GameEngine::handleMouseLeftClick(int clickX)
{
    mouseScreenX = clickX;
    mouseControl = true;

    if (gameState == GameState::PAUSED)
    {
        togglePause();
    }
}

void GameEngine::handleMouseRightClick()
{
    togglePause();
}

int GameEngine::levelForScore(int s) const
{
    return std::min(MAX_LEVEL, 1 + s / POINTS_PER_LEVEL);
}

double GameEngine::levelSpeedMultiplier() const
{
    return 1.0 + 0.20 * (level - 1);
}

void GameEngine::applyLevelSettings()
{
    basket.halfWidth = std::max(2, 6 - (level - 1));
}

void GameEngine::checkLevelUp()
{
    const int newLevel = levelForScore(score);
    if (newLevel <= level)
    {
        return;
    }

    const int oldHalfWidth = basket.halfWidth;
    level = newLevel;
    applyLevelSettings();
    levelBannerTicks = 60; // ~1.8s banner

    const double rimY = basket.mathY + basket.height;
    spawnCatchParticles(basket.mathX, rimY, QColor(199, 125, 255), 30);
    addFloatingText(basket.mathX, rimY + 8.0, QString("LEVEL %1!").arg(level), QColor(199, 125, 255));

    const bool shrank = basket.halfWidth < oldHalfWidth;
    statusMessage = QString("🚀 LEVEL %1! Eggs fall faster%2")
                        .arg(level)
                        .arg(shrank ? " and your basket got smaller!" : "!");
}

void GameEngine::updatePhysics(const MyGrid &grid)
{
    const int frameW = grid.getWidth();
    const int frameH = grid.getHeight();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int minMathY = grid.screenToMathY(frameH);

    groundMathY = minMathY + 9;
    basket.mathY = groundMathY + 1;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));
    basket.speed = std::clamp(screenSpan / 70.0, 1.6, 5.2);

    if (levelBannerTicks > 0)
    {
        levelBannerTicks--;
    }

    // 1. Move Basket smoothly
    if (keyLeftPressed)
    {
        basket.mathX -= basket.speed;
    }
    if (keyRightPressed)
    {
        basket.mathX += basket.speed;
    }

    if (mouseControl && !keyLeftPressed && !keyRightPressed)
    {
        const double targetX = static_cast<double>(mouseScreenX - grid.getOriginX()) / grid.getScale();
        const double maxStep = basket.speed * 2.0;
        basket.mathX += std::clamp(targetX - basket.mathX, -maxStep, maxStep);
    }

    const double minBasketX = minMathX + basket.halfWidth + 1.0;
    const double maxBasketX = maxMathX - basket.halfWidth - 1.0;
    if (minBasketX < maxBasketX)
    {
        basket.mathX = std::clamp(basket.mathX, minBasketX, maxBasketX);
    }

    // 2. Update Birds and Egg Laying
    updateBirds(grid);
    updateEggSpawn(grid);

    // 3. Update Falling Egg
    updateFallingEgg();

    // 4. Update Particle Effects
    updateParticles();
}

void GameEngine::updateBirds(const MyGrid &grid)
{
    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);

    for (size_t i = 0; i < birds.size(); ++i)
    {
        Bird &bird = birds[i];

        const int targetY = maxMathY - 9 - static_cast<int>(i * 4);
        bird.mathY = targetY;

        bird.wingTick++;
        if (bird.wingTick >= 6)
        {
            bird.wingTick = 0;
            bird.wingFrame = 1 - bird.wingFrame;
        }

        if (bird.isLaying)
        {
            bird.layingCountdown--;
            bird.mathX += (bird.layingCountdown % 2 == 0 ? 0.25 : -0.25);

            if (bird.layingCountdown <= 0)
            {
                fallingEgg.active = true;
                fallingEgg.mathX = bird.mathX;
                fallingEgg.mathY = bird.mathY - 2.0;
                fallingEgg.type = bird.pendingEggType;
                fallingEgg.animTick = 0;
                fallingEgg.vy = 0.08;

                const int totalHeight = std::max(30, maxMathY - groundMathY);
                const double speedMul = levelSpeedMultiplier();
                fallingEgg.gravity = (0.010 + (totalHeight / 7500.0)) * speedMul;
                fallingEgg.maxSpeed = (1.2 + (totalHeight / 80.0)) * speedMul;

                bird.isLaying = false;

                if (fallingEgg.type == EggType::GOLDEN)
                {
                    statusMessage = "✨ A bird laid a SPECIAL GOLDEN EGG! Catch it for bonus points!";
                }
                else if (fallingEgg.type == EggType::BOMB)
                {
                    statusMessage = "⚠️ A bird dropped a BOMB! Do NOT catch it in your basket!";
                }
                else
                {
                    statusMessage = "An egg is falling! Catch it!";
                }
            }
        }
        else
        {
            bird.mathX += bird.direction * bird.speed * (1.0 + 0.15 * (level - 1));

            if (bird.mathX > maxMathX - 6.0 && bird.direction > 0)
            {
                bird.direction = -1;
            }
            else if (bird.mathX < minMathX + 6.0 && bird.direction < 0)
            {
                bird.direction = 1;
            }
        }
    }
}

void GameEngine::updateEggSpawn(const MyGrid &grid)
{
    bool anyBirdLaying = false;
    for (const auto &b : birds)
    {
        if (b.isLaying)
        {
            anyBirdLaying = true;
            break;
        }
    }

    if (!fallingEgg.active && !anyBirdLaying)
    {
        eggSpawnCooldown--;
        if (eggSpawnCooldown <= 0)
        {
            const int frameW = grid.getWidth();
            const int minMathX = grid.screenToMathX(0);
            const int maxMathX = grid.screenToMathX(frameW);

            std::vector<size_t> validIndices;
            for (size_t i = 0; i < birds.size(); ++i)
            {
                if (birds[i].mathX >= minMathX + 8.0 && birds[i].mathX <= maxMathX - 8.0)
                {
                    validIndices.push_back(i);
                }
            }

            if (!validIndices.empty())
            {
                std::uniform_int_distribution<size_t> birdDist(0, validIndices.size() - 1);
                size_t chosenIdx = validIndices[birdDist(rng)];
                Bird &chosenBird = birds[chosenIdx];

                const int bombPct = std::min(50, 17 + 4 * (level - 1));
                const int goldenPct = 18;
                const int regularPct = 100 - goldenPct - bombPct;
                std::uniform_int_distribution<int> typeDist(0, 99);
                const int roll = typeDist(rng);

                EggType eType = EggType::REGULAR;
                if (roll < regularPct)
                {
                    eType = EggType::REGULAR;
                }
                else if (roll < regularPct + goldenPct)
                {
                    eType = EggType::GOLDEN;
                }
                else
                {
                    eType = EggType::BOMB;
                }

                chosenBird.isLaying = true;
                chosenBird.layingCountdown = std::max(8, 28 - 3 * (level - 1));
                chosenBird.pendingEggType = eType;

                const double gapFactor = std::max(0.25, 1.0 - 0.09 * (level - 1));
                std::uniform_int_distribution<int> cooldownDist(static_cast<int>(95 * gapFactor),
                                                                static_cast<int>(160 * gapFactor));
                eggSpawnCooldown = cooldownDist(rng);
            }
        }
    }
}

void GameEngine::updateFallingEgg()
{
    if (!fallingEgg.active)
    {
        return;
    }

    const double prevEggY = fallingEgg.mathY;
    fallingEgg.vy = std::min(fallingEgg.maxSpeed, fallingEgg.vy + fallingEgg.gravity);
    fallingEgg.mathY -= fallingEgg.vy;
    fallingEgg.animTick++;

    // 1. Collision detection with Basket Rim
    const double rimY = basket.mathY + basket.height;
    if (fallingEgg.mathY <= rimY + 0.6 && prevEggY >= basket.mathY)
    {
        const double deltaX = std::abs(fallingEgg.mathX - basket.mathX);
        if (deltaX <= basket.halfWidth + 0.8)
        {
            fallingEgg.active = false;

            if (fallingEgg.type == EggType::REGULAR)
            {
                score += 10;
                highScore = std::max(highScore, score);
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(255, 255, 230), 12);
                addFloatingText(basket.mathX, rimY + 3.0, "+10", QColor(46, 204, 113));
                statusMessage = "Caught Regular Egg! +10 pts";
            }
            else if (fallingEgg.type == EggType::GOLDEN)
            {
                score += 50;
                highScore = std::max(highScore, score);
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(255, 215, 0), 24);
                addFloatingText(basket.mathX, rimY + 3.0, "⭐ +50 BONUS! ⭐", QColor(255, 215, 0));
                statusMessage = "⭐ BONUS! Caught Golden Egg! +50 pts! ⭐";
            }
            else if (fallingEgg.type == EggType::BOMB)
            {
                spawnExplosionParticles(fallingEgg.mathX, rimY);
                addFloatingText(basket.mathX, rimY + 4.0, "💥 BOOM! 💥", QColor(231, 76, 60));
                gameOverReason = "Caught a Bomb! Instant Game Over!";
                gameState = GameState::GAME_OVER;
                statusMessage = "💥 KABOOM! You caught a bomb! Game Over!";
            }

            if (gameState == GameState::PLAYING)
            {
                checkLevelUp();
            }
            return;
        }
    }

    // 2. Collision with Ground
    if (fallingEgg.mathY <= groundMathY + 1.0)
    {
        fallingEgg.active = false;

        if (fallingEgg.type == EggType::REGULAR || fallingEgg.type == EggType::GOLDEN)
        {
            spawnSplatParticles(fallingEgg.mathX, groundMathY + 1, fallingEgg.type);
            hearts--;
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "SPLAT! -1 ❤", QColor(231, 76, 60));

            if (hearts <= 0)
            {
                hearts = 0;
                gameOverReason = "Out of Hearts! 3 misses reached.";
                gameState = GameState::GAME_OVER;
                statusMessage = "Game Over! You lost all 3 hearts.";
            }
            else
            {
                statusMessage = QString("Egg missed and cracked! Lost 1 heart (%1 left).").arg(hearts);
            }
        }
        else if (fallingEgg.type == EggType::BOMB)
        {
            spawnCatchParticles(fallingEgg.mathX, groundMathY + 1, QColor(120, 120, 120), 10);
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "DODGED BOMB! Safe!", QColor(168, 199, 250));
            statusMessage = "Bomb safely exploded on the ground! Nice dodge!";
        }
    }
}

void GameEngine::updateParticles()
{
    for (size_t i = 0; i < particles.size(); )
    {
        GameParticle &p = particles[i];
        p.x += p.vx;
        p.y += p.vy;
        p.vy -= 0.04; // gravity
        p.life--;

        if (p.life <= 0)
        {
            particles[i] = particles.back();
            particles.pop_back();
        }
        else
        {
            ++i;
        }
    }

    for (size_t i = 0; i < floatingTexts.size(); )
    {
        FloatingText &t = floatingTexts[i];
        t.y += 0.16;
        t.life--;

        if (t.life <= 0)
        {
            floatingTexts[i] = floatingTexts.back();
            floatingTexts.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

void GameEngine::spawnCatchParticles(double x, double y, const QColor &col, int count)
{
    std::uniform_real_distribution<double> speedDist(-0.7, 0.7);
    std::uniform_real_distribution<double> vyDist(0.3, 1.0);

    for (int i = 0; i < count; ++i)
    {
        GameParticle p;
        p.x = x;
        p.y = y;
        p.vx = speedDist(rng);
        p.vy = vyDist(rng);
        p.color = col;
        p.life = 16;
        p.maxLife = 16;
        particles.push_back(p);
    }
}

void GameEngine::spawnSplatParticles(double x, double y, EggType type)
{
    const QColor yolk(241, 196, 15);
    const QColor shell(245, 245, 245);
    const QColor gold(255, 215, 0);

    std::uniform_real_distribution<double> vxDist(-0.8, 0.8);
    std::uniform_real_distribution<double> vyDist(0.2, 0.8);

    const int total = 18;
    for (int i = 0; i < total; ++i)
    {
        GameParticle p;
        p.x = x;
        p.y = y;
        p.vx = vxDist(rng);
        p.vy = vyDist(rng);
        if (type == EggType::GOLDEN)
        {
            p.color = (i % 2 == 0 ? gold : QColor(255, 255, 220));
        }
        else
        {
            p.color = (i % 3 == 0 ? shell : yolk);
        }
        p.life = 20;
        p.maxLife = 20;
        particles.push_back(p);
    }
}

void GameEngine::spawnExplosionParticles(double x, double y)
{
    std::uniform_real_distribution<double> vxDist(-1.3, 1.3);
    std::uniform_real_distribution<double> vyDist(-0.4, 1.5);

    const QColor colors[] = {
        QColor(231, 76, 60),
        QColor(230, 126, 34),
        QColor(241, 196, 15),
        QColor(255, 255, 255),
        QColor(52, 73, 94)
    };

    for (int i = 0; i < 40; ++i)
    {
        GameParticle p;
        p.x = x;
        p.y = y;
        p.vx = vxDist(rng);
        p.vy = vyDist(rng);
        p.color = colors[i % 5];
        p.life = 26;
        p.maxLife = 26;
        particles.push_back(p);
    }
}

void GameEngine::addFloatingText(double x, double y, const QString &text, const QColor &col)
{
    FloatingText t;
    t.x = x;
    t.y = y;
    t.text = text;
    t.color = col;
    t.life = 28;
    floatingTexts.push_back(t);
}
