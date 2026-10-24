#include "GameEngine.h"
#include "Grid.h"
#include "Pixel.h"
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
    gameState = GameState::MENU;
    gameOverReason.clear();

    keyLeftPressed = false;
    keyRightPressed = false;
    mouseControl = false;
    level = 1;
    levelBannerTicks = 0;
    lastRestoreEggLevel = 0;
    lastGrowEggMissLevel = 0;
    legendaryBirdActive = false;
    legendaryEggDropped = false;
    legendaryAnimTick = 0;

    fallingEgg.active = false;
    particles.clear();
    floatingTexts.clear();

    const int width = std::max(grid.getWidth(), 640);
    const int height = std::max(grid.getHeight(), 480);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);
    const int minMathY = grid.screenToMathY(height);

    groundMathY = minMathY + 9;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    // Initialize Basket (Enlarged)
    basket.mathX = (minMathX + maxMathX) / 2.0;
    basket.mathY = groundMathY + 1;
    basket.halfWidth = 9;
    basket.height = 7;
    basket.speed = std::clamp(screenSpan / 70.0, 1.8, 5.5);

    birds.clear();
    applyLevelSettings(grid);

    eggSpawnCooldown = 40;
    statusMessage = "Welcome to Eggscellent Catch! Press START to play.";
}

void GameEngine::reset(const MyGrid &grid)
{
    init(grid);
    gameState = GameState::PLAYING;
}

void GameEngine::togglePause()
{
    if (gameState == GameState::GAME_OVER || gameState == GameState::MENU)
    {
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
    else if (gameState == GameState::MENU)
    {
        updateBirds(grid);
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
    // Noticeable and punchy speed progression:
    // +15% speed boost per level for early levels (1 to 10),
    // and +5% per level continuously up to level 50.
    const double earlyBoost = std::min(static_cast<double>(level - 1), 10.0) * 0.15;
    const double lateBoost = std::max(0.0, static_cast<double>(level - 11)) * 0.05;
    return 1.0 + earlyBoost + lateBoost;
}

Bird GameEngine::createBird(size_t index, const MyGrid &grid)
{
    const int frameW = std::max(grid.getWidth(), 640);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);

    const int topY = maxMathY - 11;
    const int bottomY = groundMathY + 20;
    const int availableSpan = std::max(30, topY - bottomY);
    // Noticeably larger vertical spacing between birds (18 to 28 units):
    // Lower birds fly significantly lower, leaving dramatically less time for the player
    const size_t anticipatedCount = std::max(size_t(2), birds.size() + 1);
    const int numGaps = std::max(1, static_cast<int>(anticipatedCount - 1));
    const int verticalSpacing = std::clamp(availableSpan / numGaps, 18, 28);

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    Bird b;
    // Bird direction alternates: even indices start moving right, odd moving left
    b.direction = (index % 2 == 0) ? 1 : -1;

    // Distribute birds across horizontal space so they don't start on top of each other
    const double offsetX = 18.0 + std::fmod(index * 29.0, std::max(20.0, screenSpan - 36.0));
    b.mathX = (b.direction > 0) ? (minMathX + offsetX) : (maxMathX - offsetX);

    // Vertical spacing: lower bird index = higher altitude; each new bird is noticeably lower!
    b.mathY = std::max(bottomY, topY - static_cast<int>(index * verticalSpacing));

    // Balanced, varying base speeds
    const double baseSpeeds[] = { 340.0, 420.0, 280.0, 380.0, 310.0, 450.0, 360.0, 300.0, 400.0, 330.0 };
    const double divisor = baseSpeeds[index % 10];
    b.speed = std::clamp(screenSpan / divisor, 0.35, 1.4);

    // Stagger wing animation phase
    b.wingFrame = static_cast<int>(index % 3);
    b.wingTick = static_cast<int>((index * 2) % 5);

    // Distinctive pixel plumage palette for flock variety
    struct Plumage { QColor body; QColor wing; QColor belly; };
    static const Plumage flockPlumage[] = {
        { QColor(52, 152, 219), QColor(27, 79, 114),  QColor(243, 156, 18) },  // 0: Bluebird
        { QColor(231, 76, 60),  QColor(120, 40, 31),  QColor(250, 215, 160) }, // 1: Robin
        { QColor(46, 204, 113), QColor(20, 90, 50),   QColor(249, 231, 159) }, // 2: Greenfinch
        { QColor(241, 196, 15), QColor(125, 102, 8),  QColor(255, 255, 255) }, // 3: Goldfinch
        { QColor(155, 89, 182), QColor(81, 46, 95),   QColor(235, 222, 240) }, // 4: Purple Martin
        { QColor(230, 126, 34), QColor(33, 33, 33),   QColor(254, 249, 231) }, // 5: Oriole
        { QColor(26, 188, 156), QColor(17, 120, 100), QColor(245, 176, 65) },  // 6: Teal Kingfisher
        { QColor(192, 57, 43),  QColor(100, 30, 22),  QColor(242, 215, 213) }, // 7: Cardinal
        { QColor(41, 128, 185), QColor(21, 67, 96),   QColor(248, 249, 249) }, // 8: Blue Jay
        { QColor(39, 174, 96),  QColor(22, 160, 133), QColor(244, 208, 63) }   // 9: Peacock Finch
    };
    const Plumage &p = flockPlumage[index % 10];
    b.bodyColor = p.body;
    b.wingColor = p.wing;
    b.bellyColor = p.belly;

    return b;
}

void GameEngine::applyLevelSettings(const MyGrid &grid)
{
    // Basket starts at starting width (halfWidth = 9) and shrinks with each level down to min 2
    basket.halfWidth = std::max(2, 9 - (level - 1));
    basket.height = 7;

    // Start with 1 bird; after every 5 levels, increase bird count by 1:
    // Level 1-5: 1 bird, Level 6-10: 2 birds, Level 11-15: 3 birds, ... up to 10 birds at Level 46-50.
    const size_t targetBirdCount = static_cast<size_t>(1 + (level - 1) / 5);
    while (birds.size() < targetBirdCount)
    {
        birds.push_back(createBird(birds.size(), grid));
    }
    while (birds.size() > targetBirdCount)
    {
        birds.pop_back();
    }
}

void GameEngine::checkLevelUp(const MyGrid &grid)
{
    const int newLevel = levelForScore(score);
    if (newLevel <= level)
    {
        return;
    }

    const int levelsGained = newLevel - level;
    level = newLevel;

    const int oldHalfWidth = basket.halfWidth;
    // The basket gets smaller with each level!
    basket.halfWidth = std::max(2, basket.halfWidth - levelsGained);
    levelBannerTicks = 60; // ~1.8s banner

    const double rimY = basket.mathY + basket.height;
    spawnCatchParticles(basket.mathX, rimY, QColor(199, 125, 255), 30);
    addFloatingText(basket.mathX, rimY + 8.0, QString("LEVEL %1!").arg(level), QColor(199, 125, 255));

    // Increase number of birds by one after each 5 levels
    const size_t targetBirdCount = static_cast<size_t>(1 + (level - 1) / 5);
    const size_t oldBirdCount = birds.size();
    while (birds.size() < targetBirdCount)
    {
        birds.push_back(createBird(birds.size(), grid));
    }
    const bool newBirdAdded = (birds.size() > oldBirdCount);

    const bool shrank = basket.halfWidth < oldHalfWidth;
    if (newBirdAdded)
    {
        addFloatingText(basket.mathX, rimY + 14.0, "+1 NEW BIRD!", QColor(255, 215, 0));
        statusMessage = QString("🚀 LEVEL %1! A new lower-altitude bird joined the flock!%2")
                            .arg(level)
                            .arg(shrank ? " Basket shrunk!" : "");
    }
    else
    {
        statusMessage = QString("🚀 LEVEL %1! Eggs fall faster%2")
                            .arg(level)
                            .arg(shrank ? " and your basket got smaller!" : "!");
    }
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
    basket.speed = std::clamp(screenSpan / 70.0, 1.8, 5.5);

    if (levelBannerTicks > 0)
    {
        levelBannerTicks--;
    }

    // 1. Update Birds
    updateBirds(grid);

    // 1b. Update Special Legendary Bird
    updateLegendaryBird(grid);

    // 2. Spawn and update Falling Egg
    updateEggSpawn(grid);
    updateFallingEgg(grid);

    // 3. Move Basket via Keyboard or Mouse
    if (mouseControl)
    {
        const double targetX = grid.screenToMathX(mouseScreenX);
        const double maxStep = basket.speed * 2.2;
        basket.mathX += std::clamp(targetX - basket.mathX, -maxStep, maxStep);
    }
    else
    {
        if (keyLeftPressed)
        {
            basket.mathX -= basket.speed;
        }
        if (keyRightPressed)
        {
            basket.mathX += basket.speed;
        }
    }

    const double minBasketX = minMathX + basket.halfWidth + 1.0;
    const double maxBasketX = maxMathX - basket.halfWidth - 1.0;
    if (minBasketX < maxBasketX)
    {
        basket.mathX = std::clamp(basket.mathX, minBasketX, maxBasketX);
    }

    // 4. Update Particle Effects
    updateParticles();
}

void GameEngine::updateBirds(const MyGrid &grid)
{
    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);

    const int topY = maxMathY - 11;
    const int bottomY = groundMathY + 20;
    const int availableSpan = std::max(30, topY - bottomY);
    // Noticeably larger vertical spacing between birds (18 to 28 units):
    // Gives lower birds much lower altitude and the player significantly less time
    const int numGaps = std::max(1, static_cast<int>(birds.size() - 1));
    const int verticalSpacing = (birds.size() <= 1) ? 0 : std::clamp(availableSpan / numGaps, 18, 28);

    for (size_t i = 0; i < birds.size(); ++i)
    {
        Bird &bird = birds[i];

        const int targetY = std::max(bottomY, topY - static_cast<int>(i * verticalSpacing));
        bird.mathY = targetY;

        bird.wingTick++;
        if (bird.wingTick >= 5)
        {
            bird.wingTick = 0;
            bird.wingFrame = (bird.wingFrame + 1) % 3;
        }

        if (bird.isLaying)
        {
            bird.layingCountdown--;
            bird.mathX += (bird.layingCountdown % 2 == 0 ? 0.25 : -0.25);

            if (bird.layingCountdown <= 0)
            {
                fallingEgg.active = true;
                fallingEgg.mathX = bird.mathX;
                fallingEgg.mathY = bird.mathY - 3.0;
                fallingEgg.type = bird.pendingEggType;
                fallingEgg.animTick = 0;

                const int totalHeight = std::max(30, maxMathY - groundMathY);
                const double speedMul = levelSpeedMultiplier();
                // Initial drop velocity scales with level for instant noticeable speed
                fallingEgg.vy = 0.12 * speedMul;
                fallingEgg.gravity = (0.012 + (totalHeight / 6500.0)) * speedMul;
                fallingEgg.maxSpeed = (1.5 + (totalHeight / 70.0)) * speedMul;

                bird.isLaying = false;

                if (fallingEgg.type == EggType::GOLDEN)
                {
                    statusMessage = "✨ A bird laid a SPECIAL GOLDEN EGG! Catch it for bonus points!";
                }
                else if (fallingEgg.type == EggType::BIRD_POOP)
                {
                    statusMessage = "💩 Watch out! A bird dropped POOP! Do NOT catch it in your basket!";
                }
                else if (fallingEgg.type == EggType::BASKET_GROW)
                {
                    statusMessage = "🌟 A rare GROWTH EGG is falling! Catch it to widen your basket!";
                }
                else if (fallingEgg.type == EggType::BASKET_RESTORE)
                {
                    statusMessage = "🌈 A LEGENDARY RESTORATION EGG is falling! Catch it to restore full basket size!";
                }
                else
                {
                    statusMessage = "An egg is falling! Catch it!";
                }
            }
        }
        else
        {
            bird.mathX += bird.direction * bird.speed * (1.0 + 0.05 * (level - 1));

            if (bird.mathX > maxMathX - 10.0 && bird.direction > 0)
            {
                bird.direction = -1;
            }
            else if (bird.mathX < minMathX + 10.0 && bird.direction < 0)
            {
                bird.direction = 1;
            }
        }
    }
}

void GameEngine::spawnLegendaryBird(const MyGrid &grid)
{
    const int frameW = std::max(grid.getWidth(), 640);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);

    // Guaranteed safe minimum height so it is feasible to catch that egg:
    // Basket rim is at groundMathY + 8.
    // minBirdY is groundMathY + 28, leaving at least 20 units of fall distance!
    const int minBirdY = groundMathY + 28;
    const int maxBirdY = maxMathY - 12;

    legendaryBirdActive = true;
    legendaryEggDropped = false;
    legendaryAnimTick = 0;

    // Pick entry direction: 0 = enters from left, 1 = enters from right
    std::uniform_int_distribution<int> dirDist(0, 1);
    const int dirChoice = dirDist(rng);
    const int dir = (dirChoice == 0) ? 1 : -1;

    // Pick surprise path type: 0 = SINE_WAVE, 1 = SWOOP_ARC, 2 = DIAGONAL_GLIDE, 3 = HORIZONTAL_SURGE
    std::uniform_int_distribution<int> pathDist(0, 3);
    legendaryPathType = pathDist(rng);

    // Spawn X position: off-screen
    legendaryStartX = (dir > 0) ? (minMathX - 22.0) : (maxMathX + 22.0);
    legendaryBird.mathX = legendaryStartX;
    legendaryBird.direction = dir;
    legendaryBird.isLegendary = true;
    legendaryBird.isLaying = false;
    legendaryBird.layingCountdown = 0;
    legendaryBird.pendingEggType = EggType::BASKET_RESTORE;

    // Starting height: randomized across the safe height span
    std::uniform_real_distribution<double> yDist(minBirdY + 6.0, maxBirdY);
    legendaryStartY = yDist(rng);
    legendaryBaseY = legendaryStartY;
    legendaryBird.mathY = legendaryStartY;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));
    std::uniform_real_distribution<double> spdDist(screenSpan / 280.0, screenSpan / 210.0);
    legendarySpeedX = dir * std::clamp(spdDist(rng), 0.65, 1.4);

    if (legendaryPathType == 0)
    {
        // SINE WAVE: undulating crests and troughs
        std::uniform_real_distribution<double> ampDist(6.0, 12.0);
        legendaryAmplitude = ampDist(rng);
        std::uniform_real_distribution<double> freqDist(0.06, 0.10);
        legendaryFrequency = freqDist(rng);
    }
    else if (legendaryPathType == 1)
    {
        // SWOOP ARC: parabolic dive down and swoop back up
        std::uniform_real_distribution<double> ampDist(10.0, 16.0);
        legendaryAmplitude = ampDist(rng);
    }
    else if (legendaryPathType == 2)
    {
        // DIAGONAL GLIDE with harmonic wave
        std::uniform_real_distribution<double> slopeDist(-0.20, 0.20);
        legendarySpeedY = slopeDist(rng);
    }
    else
    {
        // HORIZONTAL SURGE
        legendaryAmplitude = 3.0;
        legendaryFrequency = 0.08;
    }

    // Target egg drop X coordinate: within comfortable center 60% of the screen
    std::uniform_real_distribution<double> dropXDist(minMathX + screenSpan * 0.25, minMathX + screenSpan * 0.75);
    legendaryDropX = dropXDist(rng);

    lastRestoreEggLevel = level;
    statusMessage = "✨ SURPRISE! A MYTHICAL PHOENIX HAS ENTERED THE SKY! Catch its legendary egg!";
    addFloatingText((minMathX + maxMathX) / 2.0, maxMathY - 14.0, "✨ MYTHICAL PHOENIX! ✨", QColor(0, 229, 255));
}

void GameEngine::updateLegendaryBird(const MyGrid &grid)
{
    if (!legendaryBirdActive)
    {
        return;
    }

    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);
    const int minBirdY = groundMathY + 28;
    const int maxBirdY = maxMathY - 12;

    legendaryAnimTick++;
    legendaryBird.animTick = legendaryAnimTick;

    legendaryBird.wingTick++;
    if (legendaryBird.wingTick >= 4)
    {
        legendaryBird.wingTick = 0;
        legendaryBird.wingFrame = (legendaryBird.wingFrame + 1) % 3;
    }

    // Trailing prismatic stardust particles
    if (legendaryAnimTick % 2 == 0)
    {
        static const QColor starCols[] = {
            QColor(0, 229, 255),
            QColor(255, 215, 0),
            QColor(233, 30, 99),
            QColor(255, 255, 255),
            QColor(156, 39, 176)
        };
        GameParticle p;
        p.x = legendaryBird.mathX - legendaryBird.direction * 10.0;
        p.y = legendaryBird.mathY + (legendaryAnimTick % 3 - 1);
        p.vx = -legendaryBird.direction * 0.28;
        p.vy = ((rng() % 10) - 5) * 0.04;
        p.color = starCols[rng() % 5];
        p.life = 14;
        p.maxLife = 14;
        particles.push_back(p);
    }

    // 1. Move along curved/surprise path
    legendaryBird.mathX += legendarySpeedX;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    if (legendaryPathType == 0)
    {
        // SINE WAVE
        legendaryBird.mathY = legendaryBaseY + legendaryAmplitude * std::sin((legendaryBird.mathX - legendaryStartX) * legendaryFrequency);
    }
    else if (legendaryPathType == 1)
    {
        // SWOOP ARC: parabolic dip near screen center
        const double midX = (minMathX + maxMathX) / 2.0;
        const double normDist = (legendaryBird.mathX - midX) / (screenSpan * 0.5);
        legendaryBird.mathY = legendaryBaseY - legendaryAmplitude * (1.0 - std::min(1.0, normDist * normDist));
    }
    else if (legendaryPathType == 2)
    {
        // DIAGONAL GLIDE with harmonic wave
        legendaryBird.mathY += legendarySpeedY + 0.4 * std::sin(legendaryAnimTick * 0.1);
    }
    else
    {
        // HORIZONTAL SURGE with subtle floating bob
        legendaryBird.mathY = legendaryBaseY + legendaryAmplitude * std::sin(legendaryAnimTick * 0.08);
    }

    // Keep within feasible catching altitude
    legendaryBird.mathY = std::clamp(legendaryBird.mathY, static_cast<double>(minBirdY), static_cast<double>(maxBirdY));

    // 2. Trigger laying when reaching drop position
    if (!legendaryEggDropped && !fallingEgg.active)
    {
        const bool reachedDropX = (legendaryBird.direction > 0)
                                      ? (legendaryBird.mathX >= legendaryDropX)
                                      : (legendaryBird.mathX <= legendaryDropX);

        if (reachedDropX && !legendaryBird.isLaying)
        {
            legendaryBird.isLaying = true;
            legendaryBird.layingCountdown = 16;
        }

        if (legendaryBird.isLaying)
        {
            legendaryBird.layingCountdown--;
            if (legendaryBird.layingCountdown <= 0)
            {
                // Drop the Legendary Restoration Egg!
                fallingEgg.active = true;
                fallingEgg.mathX = legendaryBird.mathX;
                fallingEgg.mathY = legendaryBird.mathY - 3.0;
                fallingEgg.type = EggType::BASKET_RESTORE;
                fallingEgg.animTick = 0;

                const int totalHeight = std::max(30, maxMathY - groundMathY);
                const double speedMul = levelSpeedMultiplier();
                fallingEgg.vy = 0.10 * speedMul;
                fallingEgg.gravity = (0.012 + (totalHeight / 6500.0)) * speedMul;
                fallingEgg.maxSpeed = (1.5 + (totalHeight / 70.0)) * speedMul;

                legendaryEggDropped = true;
                legendaryBird.isLaying = false;

                statusMessage = "🌈 A LEGENDARY RESTORATION EGG is falling! Catch it to restore full basket size!";
                addFloatingText(legendaryBird.mathX, legendaryBird.mathY + 5.0, "🌈 LEGENDARY EGG! 🌈", QColor(0, 229, 255));
                spawnCatchParticles(legendaryBird.mathX, legendaryBird.mathY, QColor(0, 229, 255), 28);
            }
        }
    }

    // 3. Disappear after exiting screen boundaries
    const bool exitedScreen = (legendaryBird.direction > 0 && legendaryBird.mathX > maxMathX + 24.0) ||
                             (legendaryBird.direction < 0 && legendaryBird.mathX < minMathX - 24.0);
    if (exitedScreen)
    {
        legendaryBirdActive = false;
        legendaryBird.isLaying = false;
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
    if (legendaryBirdActive && legendaryBird.isLaying)
    {
        anyBirdLaying = true;
    }

    if (!fallingEgg.active && !anyBirdLaying)
    {
        eggSpawnCooldown--;
        if (eggSpawnCooldown <= 0)
        {
            const int frameW = grid.getWidth();
            const int minMathX = grid.screenToMathX(0);
            const int maxMathX = grid.screenToMathX(frameW);

            // Very rare Legendary Basket Restore Egg:
            // Spawns a Special Surprise Phoenix exclusively to fly a surprise curved path and lay the egg!
            const bool restoreDue = (level >= 10 && (level - lastRestoreEggLevel >= 10) && basket.halfWidth < 9);
            if (restoreDue && !legendaryBirdActive)
            {
                std::uniform_int_distribution<int> restoreRollDist(0, 99);
                if (restoreRollDist(rng) < 35)
                {
                    spawnLegendaryBird(grid);
                    eggSpawnCooldown = std::max(eggSpawnCooldown, 40);
                    return; // The legendary bird handles laying the legendary egg!
                }
            }

            // If legendary bird is currently active and preparing to lay, let player focus on it
            if (legendaryBirdActive && !legendaryEggDropped)
            {
                return;
            }

            std::vector<size_t> validIndices;
            for (size_t i = 0; i < birds.size(); ++i)
            {
                if (birds[i].mathX >= minMathX + 12.0 && birds[i].mathX <= maxMathX - 12.0)
                {
                    validIndices.push_back(i);
                }
            }

            if (!validIndices.empty())
            {
                std::uniform_int_distribution<size_t> birdDist(0, validIndices.size() - 1);
                size_t chosenIdx = validIndices[birdDist(rng)];
                Bird &chosenBird = birds[chosenIdx];

                // Frequency of egg laying increases with every level, with extra boosts every 3 levels (tiers)
                const int tier = (level - 1) / 3;
                const double freqDrop = 0.055 * (level - 1) + 0.05 * tier;
                const double freqFactor = std::max(0.15, 1.0 - freqDrop);
                const int minCool = std::max(5, static_cast<int>(std::round(55 * freqFactor)));
                const int maxCool = std::max(10, static_cast<int>(std::round(85 * freqFactor)));
                std::uniform_int_distribution<int> cooldownDist(minCool, maxCool);
                eggSpawnCooldown = cooldownDist(rng);

                // Standard flock birds lay Growth, Golden, Regular eggs, or Bird Poop
                EggType eType = EggType::REGULAR;

                // Rare Growth Egg: comes when basket has shrunk (halfWidth <= 6)
                // and player has not missed a growth egg within the last 4 levels
                const bool basketIsSmall = (basket.halfWidth <= 6);
                const bool growCooldownPassed = (lastGrowEggMissLevel == 0 || (level - lastGrowEggMissLevel >= 4));
                if (basketIsSmall && growCooldownPassed)
                {
                    std::uniform_int_distribution<int> growRollDist(0, 99);
                    if (growRollDist(rng) < 22)
                    {
                        eType = EggType::BASKET_GROW;
                    }
                }

                if (eType != EggType::BASKET_GROW)
                {
                    const int poopPct = std::min(45, 15 + static_cast<int>(tier * 2.0));
                    const int goldenPct = 18;
                    const int regularPct = 100 - goldenPct - poopPct;
                    std::uniform_int_distribution<int> typeDist(0, 99);
                    const int roll = typeDist(rng);

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
                        eType = EggType::BIRD_POOP;
                    }
                }

                chosenBird.isLaying = true;
                chosenBird.layingCountdown = std::max(5, static_cast<int>(std::round(22 * freqFactor)));
                chosenBird.pendingEggType = eType;
            }
        }
    }
}

void GameEngine::updateFallingEgg(const MyGrid &grid)
{
    if (!fallingEgg.active)
    {
        return;
    }

    const double prevEggY = fallingEgg.mathY;
    fallingEgg.vy = std::min(fallingEgg.maxSpeed, fallingEgg.vy + fallingEgg.gravity);
    fallingEgg.mathY -= fallingEgg.vy;
    fallingEgg.animTick++;

    const double rimY = basket.mathY + basket.height;

    // 1. Swept Collision with Basket Rim
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
            else if (fallingEgg.type == EggType::BASKET_GROW)
            {
                score += 25;
                highScore = std::max(highScore, score);
                basket.halfWidth = std::min(10, basket.halfWidth + 2); // Widen basket!
                lastGrowEggMissLevel = 0;
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(46, 204, 113), 26);
                addFloatingText(basket.mathX, rimY + 4.0, "🧺 SIZE UP! +25 🧺", QColor(46, 204, 113));
                statusMessage = "🌟 BASKET EXPANDED! Basket grew larger! +25 pts";
            }
            else if (fallingEgg.type == EggType::BASKET_RESTORE)
            {
                score += 100;
                highScore = std::max(highScore, score);
                basket.halfWidth = 9; // Full starting basket size restored!
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(0, 229, 255), 36);
                addFloatingText(basket.mathX, rimY + 4.0, "🌈 FULL RECOVERY! +100 🌈", QColor(0, 229, 255));
                statusMessage = "🌈 LEGENDARY RECOVERY! Basket restored to full starting size! +100 pts";
            }
            else if (fallingEgg.type == EggType::BIRD_POOP)
            {
                // Catching bird poop decreases hearts by one!
                hearts--;
                spawnPoopSplashParticles(fallingEgg.mathX, rimY);
                addFloatingText(basket.mathX, rimY + 4.0, "YUCK! -1 ❤", QColor(231, 76, 60));

                if (hearts <= 0)
                {
                    hearts = 0;
                    gameOverReason = "Caught Bird Poop! Lost your final heart.";
                    gameState = GameState::GAME_OVER;
                    statusMessage = "💩 Game Over! Caught bird poop and lost your final heart!";
                }
                else
                {
                    statusMessage = QString("💩 Yuck! You caught bird poop! Lost 1 heart (%1 left).").arg(hearts);
                }
            }

            if (gameState == GameState::PLAYING)
            {
                checkLevelUp(grid);
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
        else if (fallingEgg.type == EggType::BASKET_GROW)
        {
            lastGrowEggMissLevel = level;
            spawnCatchParticles(fallingEgg.mathX, groundMathY + 1, QColor(46, 204, 113), 16);
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "MISSED! COOLDOWN: 4 LVLS", QColor(231, 76, 60));
            statusMessage = QString("Growth egg broke! No growth eggs for at least 4 levels (until Level %1).").arg(level + 4);
        }
        else if (fallingEgg.type == EggType::BASKET_RESTORE)
        {
            spawnCatchParticles(fallingEgg.mathX, groundMathY + 1, QColor(0, 229, 255), 18);
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "RESTORATION MISSED", QColor(149, 165, 166));
            statusMessage = "Restoration egg broke on the ground! Keep catching!";
        }
        else if (fallingEgg.type == EggType::BIRD_POOP)
        {
            // Bird poop hit the ground - safely dodged!
            spawnPoopSplashParticles(fallingEgg.mathX, groundMathY + 1);
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "DODGED POOP! Safe!", QColor(168, 199, 250));
            statusMessage = "💩 Bird poop safely splattered on the ground! Nice dodge!";
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
        t.y += 0.25;
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

void GameEngine::spawnPoopSplashParticles(double x, double y)
{
    const QColor poopBrown(109, 76, 65);
    const QColor poopDark(78, 52, 46);
    const QColor uricWhite(245, 245, 245);
    const QColor stinkFly(139, 195, 74);

    std::uniform_real_distribution<double> vxDist(-1.1, 1.1);
    std::uniform_real_distribution<double> vyDist(0.2, 1.2);

    for (int i = 0; i < 22; ++i)
    {
        GameParticle p;
        p.x = x;
        p.y = y;
        p.vx = vxDist(rng);
        p.vy = vyDist(rng);
        if (i % 4 == 0) p.color = uricWhite;
        else if (i % 4 == 1) p.color = stinkFly;
        else if (i % 4 == 2) p.color = poopDark;
        else p.color = poopBrown;
        p.life = 18;
        p.maxLife = 18;
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
