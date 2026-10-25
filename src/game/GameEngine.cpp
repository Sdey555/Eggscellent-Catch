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
    growthEggCount = 0;
    legendaryBirdActive = false;
    legendaryEggDropped = false;
    legendaryAnimTick = 0;

    dayNightTick = 0;
    thunderstormActive = false;
    thunderstormTicks = 0;
    thunderstormCooldown = 2800 + (rng() % 1600); // 90-140s rare initial cooldown
    lightningFlashTicks = 0;
    lightningShockTimer = 0;
    lightningStrikeState = 0;
    lightningStrikeTimer = 0;
    lightningStrikeX = 0.0;

    companionBasket.active = false;
    companionBasket.ticksLeft = 0;

    fallingEggs.clear();
    stickyPuddles.clear();
    basketInStickyPuddle = false;
    cloudDropCooldownTicks = 80;
    particles.clear();
    floatingTexts.clear();

    const int width = std::max(grid.getWidth(), 640);
    const int height = std::max(grid.getHeight(), 480);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(width);
    const int minMathY = grid.screenToMathY(height);

    groundMathY = minMathY + 9;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    // Initialize Basket
    basket.mathX = (minMathX + maxMathX) / 2.0;
    basket.mathY = groundMathY + 1;
    basket.halfWidth = 9;
    basket.height = 7;
    basket.speed = std::clamp(screenSpan / 70.0, 1.8, 5.5);
    mouseScreenX = width / 2;
    lastMouseX = width / 2;
    touchActive = false;

    birds.clear();
    applyLevelSettings(grid);

    eggSpawnCooldown = 40;
    dualDropCooldownTicks = 900 + (rng() % 600);
    statusMessage = "Welcome to Eggscellent Catch! Press START to play.";
}

void GameEngine::reset(const MyGrid &grid)
{
    init(grid);
    gameState = GameState::PLAYING;
    if (controlMode == ControlMode::GYROSCOPE)
    {
        MotionSensor::instance().start();
    }
}

void GameEngine::setMode(GameMode m)
{
    mode = m;
    if (mode == GameMode::EASY)
    {
        statusMessage = "Difficulty: EASY (1 starting bird, calm skies).";
    }
    else if (mode == GameMode::MEDIUM)
    {
        statusMessage = "Difficulty: MEDIUM (2 starting birds, faster scaling, sticky poop!).";
    }
    else
    {
        statusMessage = "Difficulty: HARD (2 starting birds, fierce speed, rare dual drops, companion basket!).";
    }
}

void GameEngine::setMode(GameMode m, const MyGrid &grid)
{
    setMode(m);
    if (gameState == GameState::MENU)
    {
        basket.halfWidth = 9;
    }
    applyLevelSettings(grid);
}

void GameEngine::cycleMode(const MyGrid &grid)
{
    if (mode == GameMode::EASY)
    {
        setMode(GameMode::MEDIUM, grid);
    }
    else if (mode == GameMode::MEDIUM)
    {
        setMode(GameMode::HARD, grid);
    }
    else
    {
        setMode(GameMode::EASY, grid);
    }
}

void GameEngine::useGrowthEgg()
{
    if (gameState != GameState::PLAYING)
    {
        return;
    }

    if (growthEggCount <= 0)
    {
        statusMessage = "No Growth Eggs in inventory! Catch a green egg first.";
        return;
    }

    growthEggCount--;
    const int increase = std::max(1, static_cast<int>(std::round(basket.halfWidth * 0.25)));
    basket.halfWidth = std::min(18, basket.halfWidth + increase); // Store new 25% expanded size!
    spawnCatchParticles(basket.mathX, basket.mathY + basket.height, QColor(46, 204, 113), 24);
    addFloatingText(basket.mathX, basket.mathY + basket.height + 4.0, "+25% SIZE UP! 🧺", QColor(46, 204, 113));
    statusMessage = QString("🌟 Used 1 Growth Egg! Basket expanded +25%! (%1 in inventory)").arg(growthEggCount);
    triggerSound(SoundId::GROW_USE);
}

void GameEngine::summonCompanionBasket()
{
    if (gameState != GameState::PLAYING)
    {
        return;
    }

    if (mode != GameMode::HARD)
    {
        statusMessage = "Companion Basket is an exclusive feature for HARD mode!";
        return;
    }

    if (companionBasket.active)
    {
        const int secsLeft = (companionBasket.ticksLeft / 33) + 1;
        statusMessage = QString("Companion Basket is already active! (%1s remaining)").arg(secsLeft);
        return;
    }

    if (growthEggCount < 2)
    {
        statusMessage = QString("Need 2 Growth Eggs to summon Companion Basket! (You have %1)").arg(growthEggCount);
        return;
    }

    growthEggCount -= 2;
    companionBasket.active = true;
    companionBasket.ticksLeft = 165; // ~5.0 seconds at 30ms/tick
    companionBasket.maxTicks = 165;
    companionBasket.mathX = basket.mathX + (basket.mathX > 0 ? -16.0 : 16.0);
    companionBasket.mathY = groundMathY + 1;
    companionBasket.halfWidth = 6;
    companionBasket.height = 7;
    companionBasket.speed = 4.8;

    spawnCatchParticles(companionBasket.mathX, companionBasket.mathY + 5, QColor(0, 229, 255), 32);
    addFloatingText(companionBasket.mathX, groundMathY + 12.0, "🤖 COMPANION ACTIVE (5s)! 🤖", QColor(0, 229, 255));
    statusMessage = "🤖 COMPANION BASKET SUMMONED! Catching eggs automatically for 5 seconds!";
    triggerSound(SoundId::COMPANION_SPAWN);
}

const FallingEgg& GameEngine::getFallingEgg() const
{
    static const FallingEgg dummy{};
    for (const auto &egg : fallingEggs)
    {
        if (egg.active)
        {
            return egg;
        }
    }
    return dummy;
}

double GameEngine::getDayNightProgress() const
{
    return static_cast<double>(dayNightTick % DAY_NIGHT_CYCLE_TICKS) / DAY_NIGHT_CYCLE_TICKS;
}

double GameEngine::getNightDurationRatio() const
{
    if (mode == GameMode::EASY)
    {
        return 0.18;
    }
    else if (mode == GameMode::MEDIUM)
    {
        return 0.28;
    }
    else
    {
        return 0.42;
    }
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
        statusMessage = "Game Paused. Press ESC to Resume.";
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
        dayNightTick++;
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

void GameEngine::setControlMode(ControlMode m)
{
    controlMode = m;
    if (controlMode == ControlMode::GYROSCOPE)
    {
        MotionSensor::instance().start();
    }
    else
    {
        MotionSensor::instance().stop();
    }
    if (controlMode == ControlMode::MOUSE_KEYBOARD)
    {
        mouseControlAllowed = true;
    }
}

void GameEngine::handleMouseMove(int screenX)
{
    if (controlMode == ControlMode::VIRTUAL_BUTTONS || controlMode == ControlMode::GYROSCOPE)
    {
        return;
    }
    if (controlMode == ControlMode::MOUSE_KEYBOARD && !mouseControlAllowed)
    {
        return;
    }
    mouseScreenX = screenX;
    lastMouseX = screenX;
    mouseControl = true;
    touchActive = true;
}

void GameEngine::handleMouseLeftClick(int clickX)
{
    if (controlMode == ControlMode::VIRTUAL_BUTTONS || controlMode == ControlMode::GYROSCOPE)
    {
        return;
    }
    if (controlMode == ControlMode::MOUSE_KEYBOARD && !mouseControlAllowed)
    {
        return;
    }
    mouseScreenX = clickX;
    lastMouseX = clickX;
    mouseControl = true;
    touchActive = true;
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
    if (mode == GameMode::EASY)
    {
        const double earlyBoost = std::min(static_cast<double>(level - 1), 10.0) * 0.15;
        const double lateBoost = std::max(0.0, static_cast<double>(level - 11)) * 0.05;
        return 1.0 + earlyBoost + lateBoost;
    }
    else if (mode == GameMode::MEDIUM)
    {
        const double earlyBoost = std::min(static_cast<double>(level - 1), 10.0) * 0.20;
        const double lateBoost = std::max(0.0, static_cast<double>(level - 11)) * 0.075;
        return 1.0 + earlyBoost + lateBoost;
    }
    else
    {
        const double earlyBoost = std::min(static_cast<double>(level - 1), 10.0) * 0.25;
        const double lateBoost = std::max(0.0, static_cast<double>(level - 11)) * 0.10;
        return 1.0 + earlyBoost + lateBoost;
    }
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
    const size_t anticipatedCount = std::max(size_t(2), birds.size() + 1);
    const int numGaps = std::max(1, static_cast<int>(anticipatedCount - 1));
    const int verticalSpacing = std::clamp(availableSpan / numGaps, 18, 28);

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    Bird b;
    b.direction = (index % 2 == 0) ? 1 : -1;
    const double offsetX = 18.0 + std::fmod(index * 29.0, std::max(20.0, screenSpan - 36.0));
    b.mathX = (b.direction > 0) ? (minMathX + offsetX) : (maxMathX - offsetX);
    b.mathY = std::max(bottomY, topY - static_cast<int>(index * verticalSpacing));

    const double baseSpeeds[] = { 340.0, 420.0, 280.0, 380.0, 310.0, 450.0, 360.0, 300.0, 400.0, 330.0 };
    const double divisor = baseSpeeds[index % 10];
    b.speed = std::clamp(screenSpan / divisor, 0.35, 1.4);

    b.wingFrame = static_cast<int>(index % 3);
    b.wingTick = static_cast<int>((index * 2) % 5);

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
    basket.height = 7;

    // For Med and Hard mode, start with TWO birds! Easy starts with 1.
    const size_t baseBirds = (mode == GameMode::EASY ? 1 : 2);
    const size_t targetBirdCount = static_cast<size_t>(baseBirds + (level - 1) / 5);
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

    basket.halfWidth = std::max(2, basket.halfWidth - levelsGained);
    levelBannerTicks = 60; // ~1.8s banner

    const double rimY = basket.mathY + basket.height;
    spawnCatchParticles(basket.mathX, rimY, QColor(199, 125, 255), 30);
    statusMessage = QString("🎉 LEVEL UP! Welcome to Level %1!").arg(level);
    triggerSound(SoundId::LEVEL_UP);

    applyLevelSettings(grid);
}

void GameEngine::updateThunderstormStrike(const MyGrid &grid)
{
    if (!thunderstormActive)
    {
        lightningStrikeState = 0;
        return;
    }

    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);

    // Trigger thunder strike around middle of the 10-second storm (~tick 165)
    if (lightningStrikeState == 0 && thunderstormTicks == 165)
    {
        lightningStrikeState = 1; // Warning ionization sizzle
        lightningStrikeTimer = 26; // ~0.8s warning
        std::uniform_real_distribution<double> offsetDist(-10.0, 10.0);
        lightningStrikeX = std::clamp(basket.mathX + offsetDist(rng),
                                      minMathX + basket.halfWidth + 4.0,
                                      maxMathX - basket.halfWidth - 4.0);
        statusMessage = "⚡ DANGER! Static ionization detected on ground! DODGE!";
        addWarningFloatingText(lightningStrikeX, groundMathY + 8.0, "⚡ DANGER! ⚡", QColor(255, 235, 59));
    }
    else if (lightningStrikeState == 1)
    {
        lightningStrikeTimer--;

        // Crackling electric ground sparks at strike location
        if (lightningStrikeTimer % 3 == 0)
        {
            GameParticle p;
            p.x = lightningStrikeX + ((rng() % 8) - 4);
            p.y = groundMathY + 1 + (rng() % 3);
            p.vx = ((rng() % 10) - 5) * 0.05;
            p.vy = 0.4 + (rng() % 10) * 0.04;
            p.color = (rng() % 2 == 0) ? QColor(0, 229, 255) : QColor(255, 235, 59);
            p.life = 12;
            p.maxLife = 12;
            particles.push_back(p);
        }

        if (lightningStrikeTimer <= 0)
        {
            // THUNDER IMPACT!
            lightningStrikeState = 2; // Striking bolt
            lightningStrikeTimer = 8;
            lightningFlashTicks = 5; // Blinding screen flash
            triggerSound(SoundId::THUNDER);

            spawnExplosionParticles(lightningStrikeX, groundMathY + 2.0);

            // Check hit on Basket
            const double delta = std::abs(basket.mathX - lightningStrikeX);
            if (delta <= basket.halfWidth + 2.5)
            {
                // Basket was hit by thunder strike!
                // Player can revive ONLY if they have all three hearts left!
                if (hearts == 3)
                {
                    // Revives!
                    hearts = 1;
                    lightningShockTimer = 7 * 33; // 231 ticks = 7 seconds
                    spawnCatchParticles(basket.mathX, basket.mathY + 5.0, QColor(0, 229, 255), 40);
                    addFloatingText(basket.mathX, basket.mathY + 6.0, "REVIVED! (SHOCKED 7s)", QColor(255, 235, 59));
                    statusMessage = "⚡ STRUCK BY LIGHTNING! Revived with 3 hearts! Basket shocked & slowed for 7s!";
                    triggerSound(SoundId::SHOCK);
                }
                else
                {
                    // Fatal strike!
                    hearts = 0;
                    gameOverReason = "Struck by lightning! (Need all 3 hearts to revive).";
                    gameState = GameState::GAME_OVER;
                    statusMessage = "⚡ Game Over! Struck by lightning without all 3 hearts to survive!";
                    triggerSound(SoundId::GAME_OVER);
                }
            }
            else
            {
                // Dodged!
                addFloatingText(lightningStrikeX, groundMathY + 4.0, "LIGHTNING DODGED! ⚡", QColor(0, 229, 255));
                statusMessage = "⚡ THUNDER STRIKE! You narrowly dodged the lethal lightning bolt!";
            }
        }
    }
    else if (lightningStrikeState == 2)
    {
        lightningStrikeTimer--;
        if (lightningStrikeTimer <= 0)
        {
            lightningStrikeState = 0;
        }
    }
}

void GameEngine::updateWeather()
{
    if (thunderstormActive)
    {
        thunderstormTicks--;
        if (thunderstormTicks <= 0)
        {
            thunderstormActive = false;
            statusMessage = "The thunderstorm has cleared! Skies are returning to calm.";
        }

        // Falling slanted rain particles during thunderstorm
        for (int r = 0; r < 5; ++r)
        {
            GameParticle p;
            p.x = basket.mathX + ((rng() % 180) - 90);
            p.y = groundMathY + 45 + (rng() % 25);
            p.vx = -0.55;
            p.vy = -1.95;
            p.color = QColor(180, 215, 255, 175);
            p.life = 28;
            p.maxLife = 28;
            particles.push_back(p);
        }

        if (lightningFlashTicks > 0)
        {
            lightningFlashTicks--;
        }
    }
    else
    {
        if (lightningFlashTicks > 0)
        {
            lightningFlashTicks--;
        }

        thunderstormCooldown--;
        if (thunderstormCooldown <= 0)
        {
            // Rare thunderstorm: cooldown 90-150s
            thunderstormCooldown = 2800 + (rng() % 1800);
            int triggerChance = 4; // Easy: rare 4%
            if (mode == GameMode::MEDIUM)
            {
                triggerChance = 8; // Medium: 8%
            }
            else if (mode == GameMode::HARD)
            {
                triggerChance = 15; // Hard: 15%
            }

            if (static_cast<int>(rng() % 100) < triggerChance)
            {
                thunderstormActive = true;
                thunderstormTicks = 330; // exactly 10 seconds!
                statusMessage = "⚡ THUNDERSTORM! Pouring rain and lightning! Birds have fled to the clouds!";
                addWarningFloatingText(basket.mathX, groundMathY + 28.0, "⚡ THUNDERSTORM (10s)! ⚡", QColor(255, 235, 59));
            }
        }
    }
}

void GameEngine::updateCompanionBasket(const MyGrid &grid)
{
    if (!companionBasket.active)
    {
        return;
    }

    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);

    companionBasket.ticksLeft--;
    if (companionBasket.ticksLeft <= 0)
    {
        companionBasket.active = false;
        statusMessage = "Companion Basket has expired.";
        return;
    }

    double targetX = basket.mathX;
    bool foundTarget = false;
    double lowestY = 99999.0;

    for (const auto &egg : fallingEggs)
    {
        if (egg.active && egg.type != EggType::BIRD_POOP)
        {
            if (egg.mathY < lowestY)
            {
                lowestY = egg.mathY;
                targetX = egg.mathX;
                foundTarget = true;
            }
        }
    }

    if (!foundTarget)
    {
        targetX = basket.mathX + (basket.mathX > 0 ? -15.0 : 15.0);
    }

    const double deltaX = targetX - companionBasket.mathX;
    const double step = std::clamp(deltaX, -companionBasket.speed, companionBasket.speed);
    companionBasket.mathX += step;

    const double minCompX = static_cast<double>(minMathX);
    const double maxCompX = static_cast<double>(maxMathX);
    if (minCompX < maxCompX)
    {
        companionBasket.mathX = std::clamp(companionBasket.mathX, minCompX, maxCompX);
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

    dayNightTick++;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));
    double effectiveSpeed = std::clamp(screenSpan / 70.0, 1.8, 5.5);

    // Update Sticky Puddles from Bird Poop (Medium & Hard difficulty)
    basketInStickyPuddle = false;
    for (size_t i = 0; i < stickyPuddles.size(); )
    {
        stickyPuddles[i].lifeTicks--;
        if (stickyPuddles[i].lifeTicks <= 0)
        {
            stickyPuddles[i] = stickyPuddles.back();
            stickyPuddles.pop_back();
        }
        else
        {
            const double dist = std::abs(basket.mathX - stickyPuddles[i].mathX);
            if (dist <= basket.halfWidth + stickyPuddles[i].radius)
            {
                basketInStickyPuddle = true;
            }
            ++i;
        }
    }

    if (basketInStickyPuddle)
    {
        effectiveSpeed *= 0.35; // Heavy 65% slowdown when walking through sticky poop puddles!
        if (dayNightTick % 4 == 0)
        {
            GameParticle p;
            p.x = basket.mathX + ((rng() % (basket.halfWidth * 2 + 1)) - basket.halfWidth);
            p.y = basket.mathY + 1;
            p.vx = ((rng() % 6) - 3) * 0.05;
            p.vy = 0.20;
            p.color = QColor(109, 76, 65, 220); // Gooey brown mud/poop splatter
            p.life = 12;
            p.maxLife = 12;
            particles.push_back(p);
        }
    }

    // After revival from lightning, basket is slower for 7 seconds!
    if (lightningShockTimer > 0)
    {
        lightningShockTimer--;
        effectiveSpeed *= 0.50; // 50% speed penalty for 7 seconds!
        if (dayNightTick % 4 == 0)
        {
            GameParticle p;
            p.x = basket.mathX + ((rng() % (basket.halfWidth * 2 + 1)) - basket.halfWidth);
            p.y = basket.mathY + (rng() % (basket.height + 1));
            p.vx = ((rng() % 10) - 5) * 0.04;
            p.vy = 0.20;
            p.color = (rng() % 2 == 0) ? QColor(0, 229, 255) : QColor(255, 235, 59);
            p.life = 8;
            p.maxLife = 8;
            particles.push_back(p);
        }
    }

    basket.speed = effectiveSpeed;

    if (levelBannerTicks > 0)
    {
        levelBannerTicks--;
    }

    updateWeather();
    updateThunderstormStrike(grid);
    updateBirds(grid);
    updateLegendaryBird(grid);
    updateCompanionBasket(grid);
    updateEggSpawn(grid);
    updateFallingEggs(grid);

    // Move Player Basket via Selected Control Scheme (Speed-limited)
    const double minBasketX = static_cast<double>(minMathX);
    const double maxBasketX = static_cast<double>(maxMathX);
    const double moveSpeed = basket.speed;

    if (controlMode == ControlMode::GYROSCOPE)
    {
        gyroTilt = MotionSensor::instance().getTiltX();
        // Responsive tilt glide up to full speed
        const double gyroMul = basketInStickyPuddle ? 0.45 : 1.35;
        basket.mathX += gyroTilt * moveSpeed * gyroMul;

        // Support auxiliary keyboard arrows or on-screen button taps if pressed
        if (keyLeftPressed)
        {
            basket.mathX -= moveSpeed;
        }
        if (keyRightPressed)
        {
            basket.mathX += moveSpeed;
        }
    }
    else if (controlMode == ControlMode::VIRTUAL_BUTTONS)
    {
        if (keyLeftPressed)
        {
            basket.mathX -= moveSpeed;
        }
        if (keyRightPressed)
        {
            basket.mathX += moveSpeed;
        }
    }
    else if (controlMode == ControlMode::SCREEN_TOUCH)
    {
        if (touchActive)
        {
            const double norm = std::clamp(static_cast<double>(mouseScreenX) / std::max(1, frameW - 1), 0.0, 1.0);
            const double targetX = minBasketX + norm * (maxBasketX - minBasketX);
            const double deltaX = targetX - basket.mathX;
            if (std::abs(deltaX) < 0.6)
            {
                basket.mathX = targetX;
                touchActive = false;
            }
            else
            {
                // Strict speed limit: basket moves towards finger up to maxStep per tick
                const double maxStep = basketInStickyPuddle ? (moveSpeed * 0.65) : (moveSpeed * 1.85);
                basket.mathX += std::clamp(deltaX, -maxStep, maxStep);
            }
        }
    }
    else // ControlMode::MOUSE_KEYBOARD
    {
        if (mouseControl && mouseControlAllowed)
        {
            const double norm = std::clamp(static_cast<double>(mouseScreenX) / std::max(1, frameW - 1), 0.0, 1.0);
            const double targetX = minBasketX + norm * (maxBasketX - minBasketX);
            const double deltaX = targetX - basket.mathX;
            // Limited speed as it was previously: smooth chase up to maxStep per tick (heavily dragged in slime)
            const double maxStep = basketInStickyPuddle ? (moveSpeed * 0.70) : (moveSpeed * 2.0);
            basket.mathX += std::clamp(deltaX, -maxStep, maxStep);
        }
        else
        {
            if (keyLeftPressed)
            {
                basket.mathX -= moveSpeed;
            }
            if (keyRightPressed)
            {
                basket.mathX += moveSpeed;
            }
        }
    }

    if (minBasketX < maxBasketX)
    {
        basket.mathX = std::clamp(basket.mathX, minBasketX, maxBasketX);
    }

    updateParticles();
}

bool GameEngine::isBirdInCloud(const Bird &bird, const MyGrid &grid) const
{
    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);
    const int mathSpan = maxMathX - minMathX + 90;

    for (const auto &cd : GAME_CLOUDS)
    {
        if (!cd.isForeground)
        {
            continue;
        }
        const int cx = minMathX - 45 + ((dayNightTick / cd.speedDivider) + cd.tickOffset) % mathSpan;
        const int cy = maxMathY + cd.relY;

        const bool insideX = (bird.mathX >= cx + 4.0 && bird.mathX <= cx + cd.width - 4.0);
        const bool insideY = (bird.mathY >= cy - 2.0 && bird.mathY <= cy + cd.height + 2.0);

        if (insideX && insideY)
        {
            return true;
        }
    }
    return false;
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
    const int numGaps = std::max(1, static_cast<int>(birds.size() - 1));
    const int verticalSpacing = (birds.size() <= 1) ? 0 : std::clamp(availableSpan / numGaps, 18, 28);

    for (size_t i = 0; i < birds.size(); ++i)
    {
        Bird &bird = birds[i];
        bird.hiddenInCloud = isBirdInCloud(bird, grid);

        // While thunderstorm, birds retreat into the shelter of high storm clouds!
        int targetY;
        if (thunderstormActive)
        {
            targetY = maxMathY - 10 - static_cast<int>(i % 3);
            bird.isLaying = false; // Birds don't lay eggs while sheltering in the storm!
        }
        else
        {
            targetY = std::max(bottomY, topY - static_cast<int>(i * verticalSpacing));
        }

        // Smooth flight toward targetY
        if (bird.mathY < targetY) bird.mathY = std::min(static_cast<double>(targetY), bird.mathY + 0.6);
        else if (bird.mathY > targetY) bird.mathY = std::max(static_cast<double>(targetY), bird.mathY - 0.6);

        bird.wingTick++;
        if (bird.wingTick >= 5)
        {
            bird.wingTick = 0;
            bird.wingFrame = (bird.wingFrame + 1) % 3;
        }

        if (bird.isLaying && !thunderstormActive)
        {
            bird.layingCountdown--;
            bird.mathX += (bird.layingCountdown % 2 == 0 ? 0.25 : -0.25);

            if (bird.layingCountdown <= 0)
            {
                spawnEggFromBird(bird, grid, bird.pendingEggType);
                bird.isLaying = false;
            }
        }
        else
        {
            const double modeSpeedBoost = (mode == GameMode::HARD ? 0.08 : (mode == GameMode::MEDIUM ? 0.06 : 0.04));
            bird.mathX += bird.direction * bird.speed * (1.0 + modeSpeedBoost * (level - 1));

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

void GameEngine::spawnEggFromBird(Bird &bird, const MyGrid &grid, EggType eType)
{
    Q_UNUSED(grid);
    const int maxMathY = groundMathY + 50;
    const int totalHeight = std::max(30, maxMathY - groundMathY);
    const double speedMul = levelSpeedMultiplier();

    FallingEgg newEgg;
    newEgg.active = true;
    newEgg.mathX = bird.mathX;
    newEgg.mathY = bird.mathY - 3.0;
    newEgg.type = eType;
    newEgg.animTick = 0;
    newEgg.vy = 0.12 * speedMul;
    newEgg.gravity = (0.012 + (totalHeight / 6500.0)) * speedMul;
    newEgg.maxSpeed = (1.5 + (totalHeight / 70.0)) * speedMul;

    fallingEggs.push_back(newEgg);

    if (bird.hiddenInCloud)
    {
        addWarningFloatingText(bird.mathX, bird.mathY - 2.0, "☁️ CLOUD DROP!", QColor(168, 199, 250));
        statusMessage = "☁️ Surprise! A bird dropped an egg from inside the clouds!";
    }
    else if (eType == EggType::GOLDEN)
    {
        statusMessage = "✨ A bird laid a SPECIAL GOLDEN EGG! Catch it for bonus points!";
    }
    else if (eType == EggType::BIRD_POOP)
    {
        statusMessage = "💩 Watch out! A bird dropped POOP! Do NOT catch it in your basket!";
    }
    else if (eType == EggType::BASKET_GROW)
    {
        statusMessage = "🌟 A rare GROWTH EGG is falling! Catch it to store in inventory (Press H)!";
    }
    else if (eType == EggType::BASKET_RESTORE)
    {
        statusMessage = "🌈 A LEGENDARY RESTORATION EGG is falling! Catch it to restore full basket size!";
    }
    else
    {
        statusMessage = "An egg is falling! Catch it!";
    }
}

void GameEngine::spawnLegendaryBird(const MyGrid &grid)
{
    const int frameW = std::max(grid.getWidth(), 640);
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);
    const int maxMathY = grid.screenToMathY(0);
    const int minBirdY = groundMathY + 28;
    const int maxBirdY = maxMathY - 12;

    legendaryBirdActive = true;
    legendaryEggDropped = false;
    legendaryAnimTick = 0;

    std::uniform_int_distribution<int> dirDist(0, 1);
    const int dirChoice = dirDist(rng);
    const int dir = (dirChoice == 0) ? 1 : -1;

    std::uniform_int_distribution<int> pathDist(0, 3);
    legendaryPathType = pathDist(rng);

    std::uniform_real_distribution<double> yDist(minBirdY, maxBirdY);
    legendaryStartY = yDist(rng);
    legendaryBaseY = legendaryStartY;

    legendaryBird.bodyColor = QColor(255, 140, 0);
    legendaryBird.wingColor = QColor(229, 57, 53);
    legendaryBird.bellyColor = QColor(255, 238, 88);
    legendaryBird.direction = dir;
    legendaryBird.wingFrame = 0;
    legendaryBird.wingTick = 0;
    legendaryBird.isLaying = false;

    if (dir > 0)
    {
        legendaryStartX = minMathX - 22.0;
    }
    else
    {
        legendaryStartX = maxMathX + 22.0;
    }
    legendaryBird.mathX = legendaryStartX;
    legendaryBird.mathY = legendaryStartY;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));
    std::uniform_real_distribution<double> spdDist(screenSpan / 280.0, screenSpan / 190.0);
    legendarySpeedX = spdDist(rng) * dir;

    if (legendaryPathType == 0)
    {
        std::uniform_real_distribution<double> ampDist(7.0, 14.0);
        legendaryAmplitude = ampDist(rng);
        legendaryFrequency = 0.045 + (rng() % 30) * 0.001;
    }
    else if (legendaryPathType == 1)
    {
        std::uniform_real_distribution<double> ampDist(12.0, 18.0);
        legendaryAmplitude = ampDist(rng);
        legendaryFrequency = 3.14159 / screenSpan;
    }
    else if (legendaryPathType == 2)
    {
        std::uniform_real_distribution<double> slopeDist(-0.18, 0.18);
        legendarySpeedY = slopeDist(rng);
    }
    else
    {
        legendaryAmplitude = 3.0;
        legendaryFrequency = 0.08;
    }

    std::uniform_real_distribution<double> dropXDist(minMathX + screenSpan * 0.25, minMathX + screenSpan * 0.75);
    legendaryDropX = dropXDist(rng);

    lastRestoreEggLevel = level;
    statusMessage = "🔥 SURPRISE! A MAJESTIC FIRE PHOENIX HAS ENTERED THE SKY! Catch its legendary egg!";
    addFloatingText((minMathX + maxMathX) / 2.0, maxMathY - 14.0, "🔥 FIRE PHOENIX! 🔥", QColor(255, 140, 0));
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

    if (legendaryAnimTick % 2 == 0)
    {
        static const QColor emberCols[] = {
            QColor(255, 254, 235),
            QColor(255, 215, 0),
            QColor(255, 112, 67),
            QColor(229, 57, 53),
            QColor(255, 160, 0)
        };
        GameParticle p;
        p.x = legendaryBird.mathX - legendaryBird.direction * 10.0;
        p.y = legendaryBird.mathY + (legendaryAnimTick % 3 - 1);
        p.vx = -legendaryBird.direction * 0.28;
        p.vy = 0.08 + ((rng() % 10) - 3) * 0.03;
        p.color = emberCols[rng() % 5];
        p.life = 14;
        p.maxLife = 14;
        particles.push_back(p);
    }

    legendaryBird.mathX += legendarySpeedX;
    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    if (legendaryPathType == 0)
    {
        legendaryBird.mathY = legendaryBaseY + legendaryAmplitude * std::sin((legendaryBird.mathX - legendaryStartX) * legendaryFrequency);
    }
    else if (legendaryPathType == 1)
    {
        const double progress = std::clamp(std::abs(legendaryBird.mathX - legendaryStartX) / screenSpan, 0.0, 1.0);
        legendaryBird.mathY = legendaryBaseY - legendaryAmplitude * std::sin(progress * 3.14159);
    }
    else if (legendaryPathType == 2)
    {
        legendaryBird.mathY += legendarySpeedY;
    }
    else
    {
        legendaryBird.mathY = legendaryBaseY + legendaryAmplitude * std::sin(legendaryAnimTick * legendaryFrequency);
    }

    legendaryBird.mathY = std::clamp(legendaryBird.mathY, static_cast<double>(minBirdY), static_cast<double>(maxBirdY));

    if (!legendaryEggDropped)
    {
        const bool reachedDropZone = (legendaryBird.direction > 0 && legendaryBird.mathX >= legendaryDropX) ||
                                     (legendaryBird.direction < 0 && legendaryBird.mathX <= legendaryDropX);
        if (reachedDropZone)
        {
            legendaryEggDropped = true;
            legendaryBird.isLaying = true;

            const int totalHeight = std::max(30, maxMathY - groundMathY);
            const double speedMul = levelSpeedMultiplier();

            FallingEgg newEgg;
            newEgg.active = true;
            newEgg.mathX = legendaryBird.mathX;
            newEgg.mathY = legendaryBird.mathY - 3.0;
            newEgg.type = EggType::BASKET_RESTORE;
            newEgg.animTick = 0;
            newEgg.vy = 0.10 * speedMul;
            newEgg.gravity = (0.012 + (totalHeight / 6500.0)) * speedMul;
            newEgg.maxSpeed = (1.5 + (totalHeight / 70.0)) * speedMul;
            fallingEggs.push_back(newEgg);

            statusMessage = "🌈 THE PHOENIX DROPPED THE LEGENDARY RESTORATION EGG! Catch it!";
            addFloatingText(legendaryBird.mathX, legendaryBird.mathY + 4.0, "🌈 LEGENDARY EGG! 🌈", QColor(0, 229, 255));
        }
    }

    const bool exited = (legendaryBird.direction > 0 && legendaryBird.mathX > maxMathX + 35.0) ||
                        (legendaryBird.direction < 0 && legendaryBird.mathX < minMathX - 35.0);
    if (exited)
    {
        legendaryBirdActive = false;
    }
}

void GameEngine::updateEggSpawn(const MyGrid &grid)
{
    // If thunderstorm is currently active, birds hide and do not lay eggs!
    if (thunderstormActive)
    {
        return;
    }

    const int frameW = grid.getWidth();
    const int minMathX = grid.screenToMathX(0);
    const int maxMathX = grid.screenToMathX(frameW);

    bool anyBirdLaying = false;
    for (const auto &b : birds)
    {
        if (b.isLaying)
        {
            anyBirdLaying = true;
            break;
        }
    }

    bool canSpawn = (fallingEggs.empty() && !anyBirdLaying);
    if (mode == GameMode::HARD && fallingEggs.size() < 2 && !anyBirdLaying)
    {
        canSpawn = true;
    }

    if (canSpawn)
    {
        const int tier = (level - 1) / 3;
        auto pickEggType = [this, tier]() -> EggType {
            // Green Growth Egg probability dynamically scales up as basket size decreases!
            const int baseGrowPct = (mode == GameMode::HARD ? 6 : (mode == GameMode::MEDIUM ? 8 : 10));
            const int sizeDeficit = std::max(0, 9 - basket.halfWidth); // Increases as basket shrinks from 9 down to 2
            const int growBonus = sizeDeficit * (mode == GameMode::HARD ? 5 : (mode == GameMode::MEDIUM ? 5 : 6));
            const int maxGrowPct = (mode == GameMode::HARD ? 42 : (mode == GameMode::MEDIUM ? 46 : 50));
            const int growChance = std::min(maxGrowPct, baseGrowPct + growBonus);

            std::uniform_int_distribution<int> growRollDist(0, 99);
            if (growRollDist(rng) < growChance)
            {
                return EggType::BASKET_GROW;
            }

            int goldenPct = (mode == GameMode::HARD ? 10 : (mode == GameMode::MEDIUM ? 14 : 18));
            int poopPct = (mode == GameMode::HARD ? std::min(60, 28 + tier * 3)
                                                 : (mode == GameMode::MEDIUM ? std::min(50, 20 + tier * 2)
                                                                             : std::min(45, 15 + tier * 2)));

            const int regularPct = std::max(10, 100 - goldenPct - poopPct);
            std::uniform_int_distribution<int> typeDist(0, 99);
            const int roll = typeDist(rng);

            if (roll < regularPct)
            {
                return EggType::REGULAR;
            }
            else if (roll < regularPct + goldenPct)
            {
                return EggType::GOLDEN;
            }
            else
            {
                return EggType::BIRD_POOP;
            }
        };

        // Rare Cloud Drop: occasionally a bird hidden inside a cloud drops an egg without warning!
        if (cloudDropCooldownTicks > 0)
        {
            cloudDropCooldownTicks--;
        }
        else if (!anyBirdLaying && fallingEggs.size() < 2)
        {
            for (size_t i = 0; i < birds.size(); ++i)
            {
                Bird &b = birds[i];
                if (b.hiddenInCloud && b.mathX >= minMathX + 12.0 && b.mathX <= maxMathX - 12.0)
                {
                    b.isLaying = true;
                    b.layingCountdown = 8;
                    b.pendingEggType = pickEggType();
                    cloudDropCooldownTicks = 650 + (rng() % 500); // 20 - 35s
                    break;
                }
            }
        }

        if (dualDropCooldownTicks > 0)
        {
            dualDropCooldownTicks--;
        }

        eggSpawnCooldown--;
        if (eggSpawnCooldown <= 0)
        {
            const bool restoreDue = (level >= 10 && (level - lastRestoreEggLevel >= 10) && basket.halfWidth < 9);
            if (restoreDue && !legendaryBirdActive)
            {
                int restoreChance = (mode == GameMode::HARD ? 18 : (mode == GameMode::MEDIUM ? 25 : 35));
                std::uniform_int_distribution<int> restoreRollDist(0, 99);
                if (restoreRollDist(rng) < restoreChance)
                {
                    spawnLegendaryBird(grid);
                    eggSpawnCooldown = std::max(eggSpawnCooldown, 40);
                    return;
                }
            }

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
                const double freqMultiplier = (mode == GameMode::HARD ? 1.4 : (mode == GameMode::MEDIUM ? 1.2 : 1.0));
                const double freqDrop = (0.055 * (level - 1) + 0.05 * tier) * freqMultiplier;
                const double freqFactor = std::max(0.12, 1.0 - freqDrop);
                const int minCool = std::max(5, static_cast<int>(std::round(55 * freqFactor)));
                const int maxCool = std::max(10, static_cast<int>(std::round(85 * freqFactor)));
                std::uniform_int_distribution<int> cooldownDist(minCool, maxCool);
                eggSpawnCooldown = cooldownDist(rng);

                bool doDualDrop = (mode == GameMode::HARD && validIndices.size() >= 2 && dualDropCooldownTicks <= 0 && (rng() % 100 < 3));

                if (doDualDrop)
                {
                    dualDropCooldownTicks = 1200 + (rng() % 800); // Rare cooldown: 36 - 60 seconds
                    size_t idxA = validIndices[rng() % validIndices.size()];
                    size_t idxB = idxA;
                    for (int attempt = 0; attempt < 5 && idxB == idxA; ++attempt)
                    {
                        idxB = validIndices[rng() % validIndices.size()];
                    }

                    Bird &birdA = birds[idxA];
                    birdA.isLaying = true;
                    birdA.layingCountdown = std::max(4, static_cast<int>(std::round(18 * freqFactor)));
                    birdA.pendingEggType = pickEggType();

                    if (idxB != idxA)
                    {
                        Bird &birdB = birds[idxB];
                        birdB.isLaying = true;
                        birdB.layingCountdown = std::max(4, static_cast<int>(std::round(22 * freqFactor)));
                        birdB.pendingEggType = pickEggType();
                    }

                    statusMessage = "⚠️ DUAL DROP ALERT! Two eggs are dropping simultaneously!";
                    addWarningFloatingText(basket.mathX, groundMathY + 24.0, "⚠️ DUAL DROP! ⚠️", QColor(255, 87, 34));
                }
                else
                {
                    std::uniform_int_distribution<size_t> birdDist(0, validIndices.size() - 1);
                    size_t chosenIdx = validIndices[birdDist(rng)];
                    Bird &chosenBird = birds[chosenIdx];

                    chosenBird.isLaying = true;
                    chosenBird.layingCountdown = std::max(5, static_cast<int>(std::round(22 * freqFactor)));
                    chosenBird.pendingEggType = pickEggType();
                }
            }
        }
    }
}

void GameEngine::updateFallingEggs(const MyGrid &grid)
{
    const double rimY = basket.mathY + basket.height;
    const double compRimY = companionBasket.mathY + companionBasket.height;

    for (size_t i = 0; i < fallingEggs.size(); )
    {
        FallingEgg &egg = fallingEggs[i];
        if (!egg.active)
        {
            fallingEggs[i] = fallingEggs.back();
            fallingEggs.pop_back();
            continue;
        }

        const double prevEggY = egg.mathY;
        egg.vy = std::min(egg.maxSpeed, egg.vy + egg.gravity);
        egg.mathY -= egg.vy;
        egg.animTick++;

        if (thunderstormActive)
        {
            egg.mathX += ((egg.animTick % 4 < 2) ? -0.10 : 0.05);
        }

        bool handled = false;

        // 1. Collision with Player Basket
        if (egg.mathY <= rimY + 0.6 && prevEggY >= basket.mathY)
        {
            const double deltaX = std::abs(egg.mathX - basket.mathX);
            if (deltaX <= basket.halfWidth + 0.8)
            {
                egg.active = false;
                handled = true;

                if (egg.type == EggType::REGULAR)
                {
                    score += 10;
                    highScore = std::max(highScore, score);
                    spawnCatchParticles(egg.mathX, rimY, QColor(255, 255, 230), 12);
                    addFloatingText(basket.mathX, rimY + 3.0, "+10", QColor(46, 204, 113));
                    statusMessage = "Caught Regular Egg! +10 pts";
                    triggerSound(SoundId::CATCH_REGULAR);
                }
                else if (egg.type == EggType::GOLDEN)
                {
                    score += 50;
                    highScore = std::max(highScore, score);
                    spawnCatchParticles(egg.mathX, rimY, QColor(255, 215, 0), 24);
                    addFloatingText(basket.mathX, rimY + 3.0, "⭐ +50 BONUS! ⭐", QColor(255, 215, 0));
                    statusMessage = "⭐ BONUS! Caught Golden Egg! +50 pts! ⭐";
                    triggerSound(SoundId::CATCH_GOLDEN);
                }
                else if (egg.type == EggType::BASKET_GROW)
                {
                    score += 25;
                    highScore = std::max(highScore, score);
                    growthEggCount++; // Stored in inventory!
                    lastGrowEggMissLevel = 0;
                    spawnCatchParticles(egg.mathX, rimY, QColor(46, 204, 113), 26);
                    addFloatingText(basket.mathX, rimY + 4.0, "+1 GROWTH EGG (Press H)", QColor(46, 204, 113));
                    statusMessage = QString("🌟 Caught Growth Egg! Stored in inventory (%1 total). Press [H] to widen basket!").arg(growthEggCount);
                    triggerSound(SoundId::CATCH_GROW);
                }
                else if (egg.type == EggType::BASKET_RESTORE)
                {
                    score += 100;
                    highScore = std::max(highScore, score);
                    basket.halfWidth = std::max(basket.halfWidth, 9);
                    spawnCatchParticles(egg.mathX, rimY, QColor(0, 229, 255), 36);
                    addFloatingText(basket.mathX, rimY + 4.0, "🌈 FULL RECOVERY! +100 🌈", QColor(0, 229, 255));
                    statusMessage = "🌈 LEGENDARY RECOVERY! Basket restored to full starting size! +100 pts";
                    triggerSound(SoundId::CATCH_LEGENDARY);
                }
                else if (egg.type == EggType::BIRD_POOP)
                {
                    hearts--;
                    spawnPoopSplashParticles(egg.mathX, rimY);
                    addFloatingText(basket.mathX, rimY + 4.0, "YUCK! -1 ❤", QColor(231, 76, 60));

                    if (hearts <= 0)
                    {
                        hearts = 0;
                        gameOverReason = "Caught Bird Poop! Lost your final heart.";
                        gameState = GameState::GAME_OVER;
                        statusMessage = "💩 Game Over! Caught bird poop and lost your final heart!";
                        triggerSound(SoundId::GAME_OVER);
                    }
                    else
                    {
                        statusMessage = QString("💩 Yuck! You caught bird poop! Lost 1 heart (%1 left).").arg(hearts);
                        triggerSound(SoundId::POOP_HIT);
                    }
                }

                if (gameState == GameState::PLAYING)
                {
                    checkLevelUp(grid);
                }
            }
        }

        // 2. Collision with Companion Basket (Hard Mode)
        if (!handled && companionBasket.active && egg.mathY <= compRimY + 0.6 && prevEggY >= companionBasket.mathY)
        {
            const double compDeltaX = std::abs(egg.mathX - companionBasket.mathX);
            if (compDeltaX <= companionBasket.halfWidth + 0.8)
            {
                egg.active = false;
                handled = true;

                if (egg.type == EggType::REGULAR)
                {
                    score += 10;
                    highScore = std::max(highScore, score);
                    spawnCatchParticles(egg.mathX, compRimY, QColor(0, 229, 255), 14);
                    addFloatingText(companionBasket.mathX, compRimY + 3.0, "COMPANION +10", QColor(0, 229, 255));
                    statusMessage = "🤖 Companion caught Regular Egg! +10 pts";
                    triggerSound(SoundId::CATCH_REGULAR);
                }
                else if (egg.type == EggType::GOLDEN)
                {
                    score += 50;
                    highScore = std::max(highScore, score);
                    spawnCatchParticles(egg.mathX, compRimY, QColor(255, 215, 0), 24);
                    addFloatingText(companionBasket.mathX, compRimY + 3.0, "COMPANION +50 ⭐", QColor(255, 215, 0));
                    statusMessage = "🤖 Companion caught Golden Egg! +50 pts";
                    triggerSound(SoundId::CATCH_GOLDEN);
                }
                else if (egg.type == EggType::BASKET_GROW)
                {
                    score += 25;
                    highScore = std::max(highScore, score);
                    growthEggCount++;
                    spawnCatchParticles(egg.mathX, compRimY, QColor(46, 204, 113), 26);
                    addFloatingText(companionBasket.mathX, compRimY + 4.0, "COMPANION +1 GROWTH", QColor(46, 204, 113));
                    statusMessage = QString("🤖 Companion retrieved a Growth Egg! (%1 in inventory)").arg(growthEggCount);
                    triggerSound(SoundId::CATCH_GROW);
                }
                else if (egg.type == EggType::BASKET_RESTORE)
                {
                    score += 100;
                    highScore = std::max(highScore, score);
                    basket.halfWidth = std::max(basket.halfWidth, 9);
                    spawnCatchParticles(egg.mathX, compRimY, QColor(0, 229, 255), 36);
                    addFloatingText(companionBasket.mathX, compRimY + 4.0, "COMPANION FULL RECOVERY!", QColor(0, 229, 255));
                    statusMessage = "🤖 Companion caught the Legendary Restoration Egg! Basket fully restored!";
                    triggerSound(SoundId::CATCH_LEGENDARY);
                }
                else if (egg.type == EggType::BIRD_POOP)
                {
                    spawnPoopSplashParticles(egg.mathX, compRimY);
                    addFloatingText(companionBasket.mathX, compRimY + 4.0, "DEFLECTED! 🛡️", QColor(0, 229, 255));
                    statusMessage = "🤖 Companion basket deflected bird poop! Safe!";
                    triggerSound(SoundId::SPLAT);
                }

                if (gameState == GameState::PLAYING)
                {
                    checkLevelUp(grid);
                }
            }
        }

        // 3. Collision with Ground
        if (!handled && egg.mathY <= groundMathY + 1.0)
        {
            egg.active = false;
            handled = true;

            if (egg.type == EggType::REGULAR || egg.type == EggType::GOLDEN)
            {
                spawnSplatParticles(egg.mathX, groundMathY + 1, egg.type);
                hearts--;
                addFloatingText(egg.mathX, groundMathY + 3.0, "SPLAT! -1 ❤", QColor(231, 76, 60));

                if (hearts <= 0)
                {
                    hearts = 0;
                    gameOverReason = "Out of Hearts! 3 misses reached.";
                    gameState = GameState::GAME_OVER;
                    statusMessage = "Game Over! You lost all 3 hearts.";
                    triggerSound(SoundId::GAME_OVER);
                }
                else
                {
                    statusMessage = QString("Egg missed and cracked! Lost 1 heart (%1 left).").arg(hearts);
                    triggerSound(SoundId::SPLAT);
                }
            }
            else if (egg.type == EggType::BASKET_GROW)
            {
                spawnCatchParticles(egg.mathX, groundMathY + 1, QColor(46, 204, 113), 16);
                addFloatingText(egg.mathX, groundMathY + 3.0, "GROWTH EGG MISSED!", QColor(231, 76, 60));
                statusMessage = "Growth egg broke on the ground! Keep catching!";
                triggerSound(SoundId::SPLAT);
            }
            else if (egg.type == EggType::BASKET_RESTORE)
            {
                spawnCatchParticles(egg.mathX, groundMathY + 1, QColor(0, 229, 255), 18);
                addFloatingText(egg.mathX, groundMathY + 3.0, "RESTORATION MISSED", QColor(149, 165, 166));
                statusMessage = "Restoration egg broke on the ground! Keep catching!";
                triggerSound(SoundId::SPLAT);
            }
            else if (egg.type == EggType::BIRD_POOP)
            {
                spawnPoopSplashParticles(egg.mathX, groundMathY + 1);

                if (mode == GameMode::MEDIUM || mode == GameMode::HARD)
                {
                    StickyPuddle sp;
                    sp.mathX = egg.mathX;
                    sp.radius = (mode == GameMode::HARD ? 6.0 : 3.5);
                    sp.lifeTicks = 350;
                    sp.maxLifeTicks = 350;
                    stickyPuddles.push_back(sp);

                    addWarningFloatingText(egg.mathX, groundMathY + 3.0, (mode == GameMode::HARD ? "STICKY SLIME (LARGE)!" : "STICKY SLIME!"), QColor(139, 195, 74));
                    statusMessage = (mode == GameMode::HARD ? "💩 Bird poop made a LARGE sticky puddle! Avoid getting slowed down!"
                                                           : "💩 Bird poop made a sticky puddle! Avoid getting slowed down!");
                    triggerSound(SoundId::POOP_GROUND);
                }
                else
                {
                    addFloatingText(egg.mathX, groundMathY + 3.0, "DODGED POOP! Safe!", QColor(168, 199, 250));
                    statusMessage = "💩 Bird poop safely splattered on the ground! Nice dodge!";
                    triggerSound(SoundId::POOP_GROUND);
                }
            }
        }

        if (handled)
        {
            fallingEggs[i] = fallingEggs.back();
            fallingEggs.pop_back();
        }
        else
        {
            ++i;
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
        p.vy -= 0.04;
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

void GameEngine::addWarningFloatingText(double x, double y, const QString &text, const QColor &col)
{
    if (!gameAssistEnabled)
    {
        return;
    }
    addFloatingText(x, y, text, col);
}
