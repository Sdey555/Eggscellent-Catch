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

    birdAttackCooldownTicks = 280;
    basketDefending = false;
    basketDefendTicks = 0;
    basketDefendCooldown = 0;
    basket.isDefending = false;

    companionBasket.active = false;
    companionBasket.ticksLeft = 0;

    fallingEggs.clear();
    stickyPuddles.clear();
    particles.clear();
    floatingTexts.clear();

    // Boss, weather and hazard systems start fresh every run
    bossPhase = BossPhase::NONE;
    boss = BossBird();
    bossPhaseTicks = 0;
    bossBannerTicks = 0;
    bossNextVolley = 60;
    bossPattern = 0;
    bossLastPattern = 0;
    bossSweepTicks = 0;
    bossSweepVx = 0.0;
    bossShots.clear();
    bossRewardDropped = false;
    bossGale = false;
    highestBossBeaten = 0;
    levelCredit = 0;
    bossesDefeated = 0;
    screenShakeTicks = 0;
    screenShakeMag = 0;

    windStrength = 0.0;
    windTarget = 0.0;
    windForced = false;
    windPhase = WindPhase::CALM;
    windPhaseTicks = 500 + static_cast<int>(rng() % 300);
    rainActive = false;
    rainTicks = 0;
    rainCooldown = 1100 + static_cast<int>(rng() % 700);
    wetness = 0.0;
    fogDensity = 0.0;
    fogAnnounced = false;
    basketSlideVel = 0.0;
    basketSlipAlpha = 1.0;
    mudCooldown = 240;
    frostCooldown = 200;
    basketOnPatch = false;
    basketPatchType = PatchType::SLIME;

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
    basket.halfWidth = 10;
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
    basket.halfWidth = std::min(12, basket.halfWidth + 2); // Widen basket!
    spawnCatchParticles(basket.mathX, basket.mathY + basket.height, QColor(46, 204, 113), 24);
    addFloatingText(basket.mathX, basket.mathY + basket.height + 4.0, "🧺 SIZE UP! 🧺", QColor(46, 204, 113));
    statusMessage = QString("🌟 Used 1 Growth Egg! Basket expanded! (%1 in inventory)").arg(growthEggCount);
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

void GameEngine::defend()
{
    if (gameState != GameState::PLAYING)
    {
        return;
    }

    if (basketDefendCooldown > 0)
    {
        return;
    }

    basketDefending = true;
    basketDefendTicks = 25;      // ~0.8s active block duration
    basketDefendCooldown = 45;   // ~1.4s cooldown
    basket.isDefending = true;

    spawnCatchParticles(basket.mathX, basket.mathY + basket.height + 2.0, QColor(0, 229, 255), 18);
    triggerSound(SoundId::UI_CLICK);
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
    int l = 1;
    while (l < MAX_LEVEL && s >= levelStartScore(l + 1))
    {
        ++l;
    }
    return l;
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
    // Progress is frozen while a boss is incoming / fighting; points still count and
    // any levels earned are cashed in the moment the boss falls.
    if (bossPhase != BossPhase::NONE)
    {
        return;
    }

    int newLevel = levelForScore(getProgressScore());
    if (newLevel <= level)
    {
        return;
    }

    // Never skip past an undefeated boss level: stop there and trigger the fight.
    bool startsBoss = false;
    for (int l = level + 1; l <= newLevel; ++l)
    {
        if (isBossLevel(l) && l > highestBossBeaten)
        {
            newLevel = l;
            startsBoss = true;
            break;
        }
    }

    const int levelsGained = newLevel - level;
    const int oldShrinkTier = (level - 1) / 2;
    const int newShrinkTier = (newLevel - 1) / 2;
    const int shrinkAmount = newShrinkTier - oldShrinkTier;
    if (shrinkAmount > 0)
    {
        basket.halfWidth = std::max(6, basket.halfWidth - shrinkAmount);
    }
    level = newLevel;
    levelBannerTicks = 60; // ~1.8s banner

    const double rimY = basket.mathY + basket.height;
    spawnCatchParticles(basket.mathX, rimY, QColor(199, 125, 255), 30);
    statusMessage = QString("🎉 LEVEL UP! Welcome to Level %1!").arg(level);
    triggerSound(SoundId::LEVEL_UP);

    applyLevelSettings(grid);

    if (startsBoss)
    {
        bossPhase = BossPhase::PENDING;
        bossPhaseTicks = 0;
        statusMessage = "⚠️ The sky falls silent... something enormous is approaching!";
    }
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
        addFloatingText(lightningStrikeX, groundMathY + 8.0, "⚡ DANGER! ⚡", QColor(255, 235, 59));
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

        // Rain showers: lighter than a thunderstorm, slanted by the wind
        if (rainActive)
        {
            for (int r = 0; r < 3; ++r)
            {
                GameParticle p;
                p.x = basket.mathX + ((rng() % 180) - 90);
                p.y = groundMathY + 40 + (rng() % 25);
                p.vx = -0.30 + windStrength * 0.75;
                p.vy = -1.75;
                p.color = QColor(180, 215, 255, 150);
                p.life = 26;
                p.maxLife = 26;
                particles.push_back(p);
            }
        }

        if (bossPhase == BossPhase::NONE)
        {
            thunderstormCooldown--;
        }
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
                addFloatingText(basket.mathX, groundMathY + 28.0, "⚡ THUNDERSTORM (10s)! ⚡", QColor(255, 235, 59));
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

    // Ground hazards (slime, mud, ice): aging, melting and their effect on the basket
    const PatchEffect patchFx = updatePatches(grid);
    effectiveSpeed *= patchFx.speedMul;

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

    // Active Basket Defense Timers
    if (basketDefendTicks > 0)
    {
        basketDefendTicks--;
        basketDefending = (basketDefendTicks > 0);
    }
    if (basketDefendCooldown > 0)
    {
        basketDefendCooldown--;
    }
    basket.isDefending = basketDefending;

    basket.speed = effectiveSpeed;

    if (levelBannerTicks > 0)
    {
        levelBannerTicks--;
    }

    if (screenShakeTicks > 0)
    {
        if (--screenShakeTicks == 0)
        {
            screenShakeMag = 0;
        }
    }

    updateWeather();
    updateWeatherEffects(grid);
    updateThunderstormStrike(grid);
    updateBoss(grid);
    updateBirds(grid);
    updateLegendaryBird(grid);
    updateCompanionBasket(grid);
    updateEggSpawn(grid);
    updateFallingEggs(grid);

    // Move Player Basket via Selected Control Scheme (Speed-limited)
    const double minBasketX = static_cast<double>(minMathX);
    const double maxBasketX = static_cast<double>(maxMathX);
    const double moveSpeed = basket.speed;
    const double basketXBeforeMove = basket.mathX;

    if (controlMode == ControlMode::GYROSCOPE)
    {
        gyroTilt = MotionSensor::instance().getTiltX();
        // Responsive tilt glide up to full speed
        basket.mathX += gyroTilt * moveSpeed * 1.35;

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
                const double maxStep = moveSpeed * 1.85;
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
            // Limited speed as it was previously: smooth chase up to maxStep per tick
            const double maxStep = moveSpeed * 2.0;
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

    // Slippery surfaces (rain-soaked ground, ice): the basket carries momentum
    // instead of stopping dead, so you have to brake early.
    basketSlipAlpha = std::min(1.0 - 0.55 * wetness, patchFx.slipAlpha);
    if (basketSlipAlpha < 0.999)
    {
        const double intended = basket.mathX - basketXBeforeMove;
        basketSlideVel += (intended - basketSlideVel) * basketSlipAlpha;
        basket.mathX = basketXBeforeMove + basketSlideVel;

        if (std::abs(basketSlideVel) > 0.9 && dayNightTick % 2 == 0)
        {
            GameParticle p;
            const bool onIce = basketOnPatch && basketPatchType == PatchType::ICE;
            p.x = basket.mathX - (basketSlideVel > 0 ? 1.0 : -1.0) * (basket.halfWidth - 1) + ((rng() % 5) - 2);
            p.y = basket.mathY;
            p.vx = -basketSlideVel * 0.18;
            p.vy = 0.28 + (rng() % 5) * 0.05;
            p.color = onIce ? QColor(214, 245, 255, 230) : QColor(150, 195, 240, 190);
            p.life = 9;
            p.maxLife = 9;
            particles.push_back(p);
        }
    }
    else
    {
        basketSlideVel = basket.mathX - basketXBeforeMove;
    }

    if (minBasketX < maxBasketX)
    {
        basket.mathX = std::clamp(basket.mathX, minBasketX, maxBasketX);
    }

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
    const int numGaps = std::max(1, static_cast<int>(birds.size() - 1));
    const int verticalSpacing = (birds.size() <= 1) ? 0 : std::clamp(availableSpan / numGaps, 18, 28);

    // -------------------------------------------------------------
    // BIRD ATTACK DISPATCHER (From Level 1 onwards!)
    // -------------------------------------------------------------
    const bool bossHides = (bossPhase == BossPhase::INTRO || bossPhase == BossPhase::FIGHT || bossPhase == BossPhase::DEFEATED);
    if (!bossHides && !thunderstormActive && !legendaryBirdActive && gameState == GameState::PLAYING)
    {
        bool anyAttacking = false;
        for (const auto &b : birds)
        {
            if (b.attackState != Bird::AttackState::NONE)
            {
                anyAttacking = true;
                break;
            }
        }

        if (!anyAttacking)
        {
            birdAttackCooldownTicks--;
            if (birdAttackCooldownTicks <= 0)
            {
                std::vector<size_t> eligible;
                for (size_t idx = 0; idx < birds.size(); ++idx)
                {
                    if (!birds[idx].isLaying && !birds[idx].inCloud && !birds[idx].inFogDive &&
                        birds[idx].mathX >= minMathX + 14.0 && birds[idx].mathX <= maxMathX - 14.0)
                    {
                        eligible.push_back(idx);
                    }
                }

                if (!eligible.empty())
                {
                    size_t chosenIdx = eligible[rng() % eligible.size()];
                    Bird &attacker = birds[chosenIdx];
                    attacker.attackState = Bird::AttackState::TELEGRAPH;
                    attacker.attackTicks = 0;
                    attacker.attackStartX = attacker.mathX;
                    attacker.attackStartY = attacker.mathY;
                    attacker.attackTargetX = basket.mathX;
                    attacker.attackTargetY = groundMathY + 1.0;
                    triggerSound(SoundId::BOSS_WARNING);
                    addFloatingText(attacker.mathX, attacker.mathY + 4.0, "⚠️ WATCH OUT! ⚠️", QColor(255, 23, 68));
                    statusMessage = "⚠️ BIRD ATTACK! Defend with [SPACE] / [DEFEND] or DODGE!";

                    const int baseInterval = std::max(180, 360 - (level - 1) * 8);
                    birdAttackCooldownTicks = baseInterval + (rng() % 90);
                }
                else
                {
                    birdAttackCooldownTicks = 45; // Retry shortly
                }
            }
        }
    }

    for (size_t i = 0; i < birds.size(); ++i)
    {
        Bird &bird = birds[i];

        // During a boss fight or storm, birds retreat and abort any pending attack
        if (bossHides || thunderstormActive)
        {
            bird.attackState = Bird::AttackState::NONE;
            bird.attackTicks = 0;
        }

        // -------------------------------------------------------------
        // ACTIVE BIRD ATTACK STATE MACHINE
        // -------------------------------------------------------------
        if (bird.attackState != Bird::AttackState::NONE)
        {
            bird.attackTicks++;

            if (bird.attackState == Bird::AttackState::TELEGRAPH)
            {
                bird.animTick++;
                if (bird.attackTicks % 3 == 0)
                {
                    bird.wingFrame = (bird.wingFrame + 1) % 3;
                }

                // Track basket position during early telegraph, then lock in
                if (bird.attackTicks < 25)
                {
                    bird.attackTargetX += (basket.mathX - bird.attackTargetX) * 0.15;
                }

                // Warning hover quiver
                bird.mathX = bird.attackStartX + (bird.attackTicks % 2 == 0 ? 0.35 : -0.35);

                if (bird.attackTicks >= 42) // ~1.3s telegraph
                {
                    bird.attackState = Bird::AttackState::DIVE;
                    bird.attackTicks = 0;
                    bird.direction = (bird.attackTargetX >= bird.mathX) ? 1 : -1;
                    triggerSound(SoundId::WIND_GUST);
                }
                continue;
            }
            else if (bird.attackState == Bird::AttackState::DIVE)
            {
                const double totalDiveTicks = 18.0;
                const double t = std::clamp(static_cast<double>(bird.attackTicks) / totalDiveTicks, 0.0, 1.0);

                bird.mathX = bird.attackStartX + (bird.attackTargetX - bird.attackStartX) * t;
                const double targetImpactY = groundMathY + 1.0;
                bird.mathY = bird.attackStartY + (targetImpactY - bird.attackStartY) * (t * t);

                if (dayNightTick % 2 == 0)
                {
                    GameParticle p;
                    p.x = bird.mathX;
                    p.y = bird.mathY + 2.0;
                    p.vx = -bird.direction * 0.12;
                    p.vy = 0.25;
                    p.color = QColor(255, 235, 59, 180);
                    p.life = 6;
                    p.maxLife = 6;
                    particles.push_back(p);
                }

                // Check collision at bottom of dive
                if (t >= 0.95 || bird.mathY <= groundMathY + 2.2)
                {
                    const double deltaX = std::abs(bird.mathX - basket.mathX);
                    const bool inBasket = (deltaX <= basket.halfWidth + 1.2);

                    if (inBasket)
                    {
                        if (basketDefending)
                        {
                            // DEFENDED! Shield deflected the attack!
                            score += 50;
                            highScore = std::max(highScore, score);
                            addFloatingText(basket.mathX, basket.mathY + basket.height + 4.0, "🛡️ DEFENDED! +50 🛡️", QColor(0, 229, 255));
                            statusMessage = "🛡️ Clang! Bird attack blocked with basket shield! +50 pts";
                            spawnCatchParticles(bird.mathX, bird.mathY, QColor(0, 229, 255), 28);
                            triggerSound(SoundId::UI_CLICK);
                        }
                        else
                        {
                            // HIT! 1 Heart Lost!
                            hearts--;
                            shake(12, 2);
                            spawnSplatParticles(bird.mathX, groundMathY + 1, EggType::BIRD_POOP);
                            triggerSound(SoundId::POOP_HIT);

                            if (hearts <= 0)
                            {
                                hearts = 0;
                                gameOverReason = "Defeated by swooping bird attack! Defend or dodge!";
                                gameState = GameState::GAME_OVER;
                                statusMessage = "Game Over! A swooping bird attacked your basket!";
                                triggerSound(SoundId::GAME_OVER);
                            }
                            else
                            {
                                addFloatingText(basket.mathX, basket.mathY + basket.height + 3.0, "💥 ATTACKED! -1 ❤", QColor(231, 76, 60));
                                statusMessage = QString("💥 Bird attacked the basket! Lost 1 heart (%1 left).").arg(hearts);
                            }
                        }
                    }
                    else
                    {
                        // DODGED!
                        score += 25;
                        highScore = std::max(highScore, score);
                        addFloatingText(bird.mathX, groundMathY + 3.0, "💨 DODGED! +25", QColor(46, 204, 113));
                        statusMessage = "💨 Swoosh! You dodged the bird's dive attack! +25 pts";
                        spawnCatchParticles(bird.mathX, groundMathY + 1, QColor(141, 110, 99), 16);
                        triggerSound(SoundId::WIND_GUST);
                    }

                    bird.attackState = Bird::AttackState::RECOVER;
                    bird.attackTicks = 0;
                    bird.attackStartX = bird.mathX;
                    bird.attackStartY = bird.mathY;
                }
                continue;
            }
            else if (bird.attackState == Bird::AttackState::RECOVER)
            {
                const double totalRecoverTicks = 26.0;
                const double t = std::clamp(static_cast<double>(bird.attackTicks) / totalRecoverTicks, 0.0, 1.0);
                const double cruiseY = std::max(bottomY, topY - static_cast<int>(i * verticalSpacing));

                bird.mathY = bird.attackStartY + (cruiseY - bird.attackStartY) * (1.0 - (1.0 - t) * (1.0 - t));
                bird.mathX += bird.direction * 0.50;

                if (t >= 1.0)
                {
                    bird.attackState = Bird::AttackState::NONE;
                    bird.attackTicks = 0;
                }
                continue;
            }
        }

        // While thunderstorm, birds retreat into the shelter of high storm clouds!
        // During a boss fight the flock bolts out of the top of the sky.
        int targetY;
        if (bossHides)
        {
            targetY = maxMathY + 30 + static_cast<int>(i % 3) * 4;
            bird.isLaying = false;
            bird.isSilhouette = false;
            bird.inFogDive = false;
            bird.inCloud = false;
        }
        else if (thunderstormActive)
        {
            targetY = maxMathY - 10 - static_cast<int>(i % 3);
            bird.isLaying = false; // Birds don't lay eggs while sheltering in the storm!
            bird.isSilhouette = false;
            bird.inFogDive = false;
            bird.inCloud = false;
        }
        else
        {
            targetY = std::max(bottomY, topY - static_cast<int>(i * verticalSpacing));
        }

        // -------------------------------------------------------------
        // FOG DIVE AT NIGHT: bird dips into fog and comes out as silhouette
        // -------------------------------------------------------------
        if (!bossHides && !thunderstormActive)
        {
            if (fogDensity > 0.12)
            {
                // Trigger occasional fog dive if not already diving or laying
                if (!bird.inFogDive && !bird.isLaying && (rng() % 200 < 2 || (!bird.isSilhouette && rng() % 50 < 2)))
                {
                    bird.inFogDive = true;
                    bird.fogDiveTicks = 0;
                    bird.fogDiveBaseY = bird.mathY;
                }

                if (bird.inFogDive)
                {
                    bird.fogDiveTicks++;
                    const double progress = static_cast<double>(bird.fogDiveTicks) / 50.0;
                    const double dip = std::sin(progress * 3.14159) * 10.0;
                    targetY = static_cast<int>(std::round(bird.fogDiveBaseY - dip));
                    bird.isSilhouette = true; // Emerges as dark silhouette

                    if (bird.fogDiveTicks >= 50)
                    {
                        bird.inFogDive = false;
                    }
                }
            }
            else if (fogDensity < 0.06 || getNightFactor() < 0.05)
            {
                // Daylight / clear air returns bird to vibrant plumage
                bird.isSilhouette = false;
                bird.inFogDive = false;
            }
        }

        // Smooth flight toward targetY
        if (bird.mathY < targetY) bird.mathY = std::min(static_cast<double>(targetY), bird.mathY + 0.6);
        else if (bird.mathY > targetY) bird.mathY = std::max(static_cast<double>(targetY), bird.mathY - 0.6);

        bird.animTick++;
        bird.wingTick++;
        if (bird.wingTick >= 5)
        {
            bird.wingTick = 0;
            bird.wingFrame = (bird.wingFrame + 1) % 3;
        }

        // -------------------------------------------------------------
        // CLOUD BANKS: bird glides into drifting cloud & disappears
        // -------------------------------------------------------------
        if (!thunderstormActive && !bossHides)
        {
            const int mathSpan = maxMathX - minMathX + 90;
            bool insideCloud = false;
            double cloudUnderY = 0.0;
            int foundCloudIdx = -1;

            for (int ci = 0; ci < NUM_CLOUD_BANKS; ++ci)
            {
                const auto &cd = getCloudDef(ci);
                const int cx = minMathX - 45 + ((dayNightTick / cd.speedDivider) + cd.tickOffset) % mathSpan;
                const int cy = maxMathY + cd.relY;

                // Cloud bounds check (bird body within cloud dome)
                if (bird.mathX >= cx + 5.0 && bird.mathX <= cx + cd.width - 5.0 &&
                    bird.mathY >= cy - 1.0 && bird.mathY <= cy + cd.height + 2.0)
                {
                    insideCloud = true;
                    cloudUnderY = cy;
                    foundCloudIdx = ci;
                    break;
                }
            }

            if (insideCloud)
            {
                if (!bird.inCloud)
                {
                    // Smoothly submerged into the cloud bank
                    spawnCloudPuffParticles(bird.mathX, bird.mathY, 6);
                }
                bird.inCloud = true;
                bird.cloudUnderbellyY = cloudUnderY;
                bird.cloudIndex = foundCloudIdx;
            }
            else
            {
                if (bird.inCloud)
                {
                    // Emerged out the other side of the cloud bank
                    spawnCloudPuffParticles(bird.mathX, bird.mathY, 5);
                }
                bird.inCloud = false;
                bird.cloudIndex = -1;
            }
        }
        else
        {
            bird.inCloud = false;
            bird.cloudIndex = -1;
        }

        if (bird.isLaying && !thunderstormActive && !bossHides)
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
    if (bird.inCloud)
    {
        // Cloud banks: bird has disappeared in cloud; egg drops out of the cloud's underside!
        newEgg.mathY = bird.cloudUnderbellyY - 1.0;
        spawnCloudPuffParticles(bird.mathX, bird.cloudUnderbellyY, 14);
    }
    else
    {
        newEgg.mathY = bird.mathY - 3.0;
    }
    newEgg.type = eType;
    newEgg.animTick = 0;
    newEgg.vy = 0.12 * speedMul;
    newEgg.gravity = (0.012 + (totalHeight / 6500.0)) * speedMul;
    newEgg.maxSpeed = (1.5 + (totalHeight / 70.0)) * speedMul;

    fallingEggs.push_back(newEgg);

    if (bird.inCloud)
    {
        statusMessage = "☁ An egg dropped out of the drifting cloud!";
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
    // A boss (incoming, fighting or leaving) also owns the sky: the flock lays nothing.
    if (thunderstormActive || bossPhase != BossPhase::NONE)
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
        if (dualDropCooldownTicks > 0)
        {
            dualDropCooldownTicks--;
        }

        eggSpawnCooldown--;
        if (eggSpawnCooldown <= 0)
        {
            const bool restoreDue = (level >= 10 && (level - lastRestoreEggLevel >= 10) && basket.halfWidth < 10);
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
                const int tier = (level - 1) / 3;
                const double freqMultiplier = (mode == GameMode::HARD ? 1.4 : (mode == GameMode::MEDIUM ? 1.2 : 1.0));
                const double freqDrop = (0.055 * (level - 1) + 0.05 * tier) * freqMultiplier;
                const double freqFactor = std::max(0.12, 1.0 - freqDrop);
                const int minCool = std::max(5, static_cast<int>(std::round(55 * freqFactor)));
                const int maxCool = std::max(10, static_cast<int>(std::round(85 * freqFactor)));
                std::uniform_int_distribution<int> cooldownDist(minCool, maxCool);
                eggSpawnCooldown = cooldownDist(rng);

                auto pickEggType = [this, tier]() -> EggType {
                    // Growth egg spawns regularly and scales up if basket has shrunk
                    int baseGrowChance = (mode == GameMode::HARD ? 14 : (mode == GameMode::MEDIUM ? 18 : 24));
                    if (growthEggCount == 0)
                    {
                        baseGrowChance += 8; // Extra boost if inventory is empty
                    }
                    const int deficit = std::max(0, 10 - basket.halfWidth);
                    const int growChance = std::min(60, baseGrowChance + deficit * 7);

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
                    addFloatingText(basket.mathX, groundMathY + 24.0, "⚠️ DUAL DROP! ⚠️", QColor(255, 87, 34));
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
    const double eggMinX = static_cast<double>(grid.screenToMathX(0)) + 2.0;
    const double eggMaxX = static_cast<double>(grid.screenToMathX(grid.getWidth())) - 2.0;

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

        // Boss volleys fan out with their own drift; wind then pushes everything sideways.
        // Droppings are heavier than eggs, and the slow trophy egg barely feels it.
        const double windMass = (egg.type == EggType::BIRD_POOP) ? 0.6 : (egg.type == EggType::BOSS_REWARD ? 0.3 : 1.0);
        egg.mathX += egg.vx + windStrength * WIND_DRIFT * windMass;
        if (eggMinX < eggMaxX)
        {
            if (egg.mathX < eggMinX)       { egg.mathX = eggMinX; egg.vx = std::abs(egg.vx) * 0.5; }
            else if (egg.mathX > eggMaxX)  { egg.mathX = eggMaxX; egg.vx = -std::abs(egg.vx) * 0.5; }
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
                    basket.halfWidth = 10;
                    spawnCatchParticles(egg.mathX, rimY, QColor(0, 229, 255), 36);
                    addFloatingText(basket.mathX, rimY + 4.0, "🌈 FULL RECOVERY! +100 🌈", QColor(0, 229, 255));
                    statusMessage = "🌈 LEGENDARY RECOVERY! Basket restored to full starting size! +100 pts";
                    triggerSound(SoundId::CATCH_LEGENDARY);
                }
                else if (egg.type == EggType::BOSS_REWARD)
                {
                    score += 100;
                    highScore = std::max(highScore, score);
                    basket.halfWidth = std::min(12, basket.halfWidth + 3);
                    QString perk;
                    if (hearts < 3)
                    {
                        hearts++;
                        perk = "+1 ❤";
                    }
                    else
                    {
                        growthEggCount += 2;
                        perk = "+2 GROWTH EGGS";
                    }
                    spawnCatchParticles(egg.mathX, rimY, QColor(255, 214, 10), 40);
                    spawnCatchParticles(egg.mathX, rimY, QColor(187, 134, 252), 24);
                    addFloatingText(basket.mathX, rimY + 5.0, QString("👑 TROPHY! +100  %1").arg(perk), QColor(255, 214, 10));
                    statusMessage = QString("👑 TROPHY EGG! Basket widened, %1, +100 pts").arg(perk);
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

                if (egg.bossShot && gameState == GameState::PLAYING)
                {
                    if (egg.type == EggType::REGULAR)     damageBoss(1, egg.mathX, rimY + 8.0);
                    else if (egg.type == EggType::GOLDEN) damageBoss(3, egg.mathX, rimY + 8.0);
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
                    basket.halfWidth = 9;
                    spawnCatchParticles(egg.mathX, compRimY, QColor(0, 229, 255), 36);
                    addFloatingText(companionBasket.mathX, compRimY + 4.0, "COMPANION FULL RECOVERY!", QColor(0, 229, 255));
                    statusMessage = "🤖 Companion caught the Legendary Restoration Egg! Basket fully restored!";
                    triggerSound(SoundId::CATCH_LEGENDARY);
                }
                else if (egg.type == EggType::BOSS_REWARD)
                {
                    score += 100;
                    highScore = std::max(highScore, score);
                    basket.halfWidth = std::min(9, basket.halfWidth + 3);
                    if (hearts < 3) hearts++; else growthEggCount += 2;
                    spawnCatchParticles(egg.mathX, compRimY, QColor(255, 214, 10), 40);
                    addFloatingText(companionBasket.mathX, compRimY + 5.0, "COMPANION: 👑 TROPHY!", QColor(255, 214, 10));
                    statusMessage = "🤖 Companion grabbed the Trophy Egg! Perks applied.";
                    triggerSound(SoundId::CATCH_LEGENDARY);
                }
                else if (egg.type == EggType::BIRD_POOP)
                {
                    spawnPoopSplashParticles(egg.mathX, compRimY);
                    addFloatingText(companionBasket.mathX, compRimY + 4.0, "DEFLECTED! 🛡️", QColor(0, 229, 255));
                    statusMessage = "🤖 Companion basket deflected bird poop! Safe!";
                    triggerSound(SoundId::SPLAT);
                }

                if (egg.bossShot && gameState == GameState::PLAYING)
                {
                    if (egg.type == EggType::REGULAR)     damageBoss(1, egg.mathX, compRimY + 8.0);
                    else if (egg.type == EggType::GOLDEN) damageBoss(3, egg.mathX, compRimY + 8.0);
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

                // On Level 3 (tier 0) boss fight: missing an egg does not cost a heart!
                // Only droppings / attacks cost hearts, so catching the boss egg is fair and accessible.
                if (egg.bossShot && boss.present && boss.tier == 0)
                {
                    addFloatingText(egg.mathX, groundMathY + 3.0, "EGG MISSED!", QColor(255, 193, 7));
                    statusMessage = "Egg missed! Catch the next one to damage Colonel Cluck!";
                    triggerSound(SoundId::SPLAT);
                }
                else
                {
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
            }
            else if (egg.type == EggType::BASKET_GROW)
            {
                spawnCatchParticles(egg.mathX, groundMathY + 1, QColor(46, 204, 113), 16);
                addFloatingText(egg.mathX, groundMathY + 3.0, "GROWTH EGG BROKE!", QColor(231, 76, 60));
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
            else if (egg.type == EggType::BOSS_REWARD)
            {
                spawnCatchParticles(egg.mathX, groundMathY + 1, QColor(255, 214, 10), 22);
                addFloatingText(egg.mathX, groundMathY + 3.0, "TROPHY EGG LOST", QColor(149, 165, 166));
                statusMessage = "The trophy egg shattered on the ground... (no penalty)";
                triggerSound(SoundId::SPLAT);
            }
            else if (egg.type == EggType::BIRD_POOP && egg.leavesPatch)
            {
                // Boss droppings always leave their signature hazard, on every difficulty
                spawnPoopSplashParticles(egg.mathX, groundMathY + 1);
                const bool ice = (egg.patchOnLand == PatchType::ICE);
                spawnPatch(egg.patchOnLand, egg.mathX, ice ? 5.0 : 4.0, ice ? 520 : 480);
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

                    addFloatingText(egg.mathX, groundMathY + 3.0, (mode == GameMode::HARD ? "STICKY SLIME (LARGE)!" : "STICKY SLIME!"), QColor(139, 195, 74));
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

void GameEngine::spawnCloudPuffParticles(double x, double y, int count)
{
    std::uniform_real_distribution<double> vxDist(-0.45, 0.45);
    std::uniform_real_distribution<double> vyDist(-0.35, -0.05);

    const double dnProg = getDayNightProgress();
    const double nr = getNightDurationRatio();
    const double nightStart = 1.0 - nr;
    const double sunsetStart = nightStart - 0.14;
    const double nightFactor = getNightFactor();

    QColor puffBase(250, 252, 255);
    QColor puffShade(210, 225, 240);

    if (thunderstormActive)
    {
        puffBase = QColor(68, 72, 88);
        puffShade = QColor(48, 52, 68);
    }
    else if (nightFactor > 0.4)
    {
        // Night slate/indigo cloud mist
        puffBase = QColor(70, 82, 105);
        puffShade = QColor(40, 48, 66);
    }
    else if (dnProg >= sunsetStart && dnProg < nightStart)
    {
        // Warm sunset amber/peach cloud mist
        puffBase = QColor(255, 185, 145);
        puffShade = QColor(225, 135, 125);
    }

    for (int i = 0; i < count; ++i)
    {
        GameParticle p;
        p.x = x + vxDist(rng) * 3.5;
        p.y = y + vyDist(rng) * 1.5;
        p.vx = vxDist(rng) * 0.4;
        p.vy = vyDist(rng);
        p.color = (i % 2 == 0) ? puffBase : puffShade;
        p.life = 16 + static_cast<int>(rng() % 8);
        p.maxLife = p.life;
        particles.push_back(p);
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

// =============================================================
// BOSS FIGHTS (level 3, then every 10 levels)
// =============================================================

void GameEngine::shake(int ticks, int mag)
{
    screenShakeTicks = std::max(screenShakeTicks, ticks);
    screenShakeMag = std::max(screenShakeMag, mag);
}

double GameEngine::bossEggSpeedMul() const
{
    // Boss eggs ride the normal difficulty curve but are capped so a volley is always readable,
    // and eased on short windows where there is little fall height to react in.
    const double fallH = bossHoverY - BOSS_BELLY_DROP - groundMathY;
    const double heightEase = std::clamp(fallH / 75.0, 0.55, 1.0);
    // On tier 0 (level 3), ease the fall speed so catching the boss egg is completely fair and readable
    const double tierEase = (boss.tier == 0) ? 0.76 : 0.92;
    return std::min(levelSpeedMultiplier(), 3.0) * tierEase * heightEase;
}

int GameEngine::estimateFallTicks(double startY, double speedMul) const
{
    // Mirrors the integration in updateFallingEggs for a boss egg
    constexpr double totalHeight = 50.0;
    double y = startY;
    double vy = 0.12 * speedMul;
    const double g = (0.012 + totalHeight / 6500.0) * speedMul;
    const double cap = (1.5 + totalHeight / 70.0) * speedMul;
    int t = 0;
    while (y > groundMathY + 1.0 && t < 600)
    {
        vy = std::min(cap, vy + g);
        y -= vy;
        ++t;
    }
    return std::max(1, t);
}

void GameEngine::updateBoss(const MyGrid &grid)
{
    if (bossBannerTicks > 0)
    {
        --bossBannerTicks;
    }

    switch (bossPhase)
    {
    case BossPhase::NONE:
        return;

    case BossPhase::PENDING:
    {
        // Let the sky settle: no storm, no phoenix, no eggs in the air, nobody mid-lay
        bool anyLaying = false;
        for (const auto &b : birds)
        {
            anyLaying = anyLaying || b.isLaying;
        }
        if (!thunderstormActive && lightningStrikeState == 0 && !legendaryBirdActive &&
            fallingEggs.empty() && !anyLaying)
        {
            startBossIntro(grid);
        }
        return;
    }

    case BossPhase::INTRO:
    {
        ++bossPhaseTicks;
        ++boss.animTick;
        static const int kFlap[4] = { 0, 1, 2, 1 };
        boss.wingFrame = kFlap[(boss.animTick / 4) % 4];

        if (boss.mathY > bossHoverY)
        {
            boss.mathY = std::max(bossHoverY, boss.mathY - 0.8);
            if (boss.mathY <= bossHoverY)
            {
                // Touchdown: the whole sky shudders
                shake(20, 3);
                for (int i = 0; i < 26; ++i)
                {
                    GameParticle p;
                    p.x = boss.mathX + (static_cast<int>(rng() % 50) - 25);
                    p.y = boss.mathY - 8 + static_cast<int>(rng() % 10);
                    p.vx = (static_cast<int>(rng() % 20) - 10) * 0.07;
                    p.vy = 0.1 + (rng() % 10) * 0.05;
                    p.color = boss.def->pal.crestLight;
                    p.life = 16;
                    p.maxLife = 16;
                    particles.push_back(p);
                }
            }
        }
        else
        {
            boss.mathY = bossHoverY + 1.6 * std::sin(boss.animTick * 0.09);
        }

        if (bossPhaseTicks >= BOSS_BANNER_TICKS)
        {
            bossPhase = BossPhase::FIGHT;
            bossPhaseTicks = 0;
            bossNextVolley = 40;
            bossTargetX = boss.mathX;
            statusMessage = QString("⚔️ %1 — catch eggs to break its health bar, dodge the droppings!")
                                .arg(boss.def->name);
        }
        return;
    }

    case BossPhase::FIGHT:
        updateBossFight(grid);
        return;

    case BossPhase::DEFEATED:
        updateBossDefeat(grid);
        return;
    }
}

void GameEngine::startBossIntro(const MyGrid &grid)
{
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());
    const int maxY = grid.screenToMathY(0);

    boss = BossBird();
    boss.present = true;
    boss.tier = bossTierForLevel(level);
    boss.def = &bossDefForTier(boss.tier);

    const double hpMul = (mode == GameMode::EASY ? 0.85 : (mode == GameMode::MEDIUM ? 1.0 : 1.2));
    boss.maxHp = std::max(6, static_cast<int>(std::round(boss.def->baseHp * hpMul)));
    boss.hp = boss.maxHp;

    bossHoverY = maxY - BOSS_HALF_HEIGHT - 19; // leaves room for the health bar above the crown
    boss.mathX = (minX + maxX) / 2.0;
    boss.mathY = maxY + BOSS_HALF_HEIGHT + 10;
    bossTargetX = boss.mathX;

    bossPhase = BossPhase::INTRO;
    bossPhaseTicks = 0;
    bossBannerTicks = BOSS_BANNER_TICKS;
    bossShots.clear();
    bossSweepTicks = 0;
    bossRewardDropped = false;
    bossGale = false;

    shake(14, 2);
    triggerSound(SoundId::BOSS_WARNING);
    statusMessage = QString("⚠️ BOSS: %1 — %2").arg(boss.def->name, boss.def->title);
}

void GameEngine::beginBossCharge(const MyGrid &grid)
{
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());
    const double span = std::max(50.0, static_cast<double>(maxX - minX));
    const double baseSpd = std::clamp(span / 70.0, 1.8, 5.5);

    // Pick a pattern the boss currently knows, avoiding an immediate repeat
    const unsigned known = boss.def->patterns | (boss.enraged ? boss.def->rageExtra : 0u);
    std::vector<unsigned> options;
    for (unsigned bit = 1; bit <= PAT_RAIN; bit <<= 1)
    {
        if ((known & bit) && (bit != bossLastPattern || __builtin_popcount(known) == 1))
        {
            options.push_back(bit);
        }
    }
    bossPattern = options[rng() % options.size()];
    bossLastPattern = bossPattern;

    const double lo = minX + 26.0;
    const double hi = maxX - 26.0;
    const double g = std::clamp(2.0 * basket.halfWidth + 5.0, 9.0, 16.0);
    const char *label = "";

    if (bossPattern == PAT_SWEEP)
    {
        // Start the strafe from the edge farthest from the player so there is room to react
        bossTargetX = (basket.mathX > (minX + maxX) / 2.0) ? lo : hi;
        label = "⇄ SWEEP! ⇄";
    }
    else
    {
        std::uniform_real_distribution<double> aim(-g, g);
        bossTargetX = std::clamp(basket.mathX + aim(rng), lo, hi);
        label = (bossPattern == PAT_FAN)   ? "▼ FAN SHOT! ▼"
              : (bossPattern == PAT_BURST) ? "▼▼ DOUBLE BURST! ▼▼"
                                           : "☔ DOWNPOUR! ☔";
    }

    // Charge long enough to actually get into position
    const double repo = baseSpd * 1.8 * boss.def->travel * (boss.enraged ? 1.2 : 1.0);
    const double dist = std::abs(bossTargetX - boss.mathX);
    int charge = static_cast<int>(std::ceil(dist / repo)) + 10;
    charge = std::clamp(charge, boss.enraged ? 16 : 24, 70);
    boss.chargeTicks = charge;
    boss.chargeMax = charge;

    addFloatingText(boss.mathX - 10.0, boss.mathY - BOSS_HALF_HEIGHT - 4.0, label, boss.def->pal.crestLight);
}

void GameEngine::fireBossVolley(const MyGrid &grid)
{
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());
    const double span = std::max(50.0, static_cast<double>(maxX - minX));
    const double baseSpd = std::clamp(span / 70.0, 1.8, 5.5);
    const double lo = minX + 8.0;
    const double hi = maxX - 8.0;
    const double minSafeGap = 2.0 * basket.halfWidth + 8.0;
    // On tier 0 (level 3), ensure generous 26-unit lane spacing so droppings never overlap the basket!
    const double g = (boss.tier == 0) ? std::max(minSafeGap, 26.0) : std::max(minSafeGap, 22.0);
    const bool rage = boss.enraged;

    auto clampX = [&](double x) { return std::clamp(x, lo, hi); };
    auto catchable = [&]() {
        const int goldenChance = rage ? 28 : 15;
        return (static_cast<int>(rng() % 100) < goldenChance) ? EggType::GOLDEN : EggType::REGULAR;
    };
    auto addFan = [&](int delay, double centre, int catchLane) {
        // Slide the whole fan inward near screen edges so no two lanes ever land on the same spot
        if (lo + 2.0 * g < hi - 2.0 * g)
        {
            centre = std::clamp(centre, lo + 2.0 * g, hi - 2.0 * g);
        }
        for (int lane = -2; lane <= 2; ++lane)
        {
            BossShot s;
            s.delay = delay;
            s.offsetX = lane * 1.6;
            s.landX = clampX(centre + lane * g);
            s.straight = false;
            s.type = (lane == catchLane) ? catchable() : EggType::BIRD_POOP;
            bossShots.push_back(s);
        }
    };

    if (bossPattern == PAT_FAN)
    {
        // Five lanes, exactly one real egg hiding among four droppings
        int chosenLane = static_cast<int>(rng() % 5) - 2;

        // For Level 3 (tier 0): guarantee that the real egg is physically reachable!
        if (boss.tier == 0)
        {
            // Pick among the central lanes {-1, 0, 1} the one closest to current basket position
            double bestDist = 99999.0;
            for (int candidateLane : { -1, 0, 1 })
            {
                const double landX = clampX(boss.mathX + candidateLane * g);
                const double dist = std::abs(landX - basket.mathX);
                if (dist < bestDist)
                {
                    bestDist = dist;
                    chosenLane = candidateLane;
                }
            }
        }

        addFan(0, boss.mathX, chosenLane);
    }
    else if (bossPattern == PAT_BURST)
    {
        // Two fans a beat apart; the second real egg is never more than one lane away
        const int laneA = static_cast<int>(rng() % 3) - 1;
        const int laneB = std::clamp(laneA + (static_cast<int>(rng() % 3) - 1), -2, 2);
        addFan(0, boss.mathX, laneA);
        addFan(rage ? 14 : 17, boss.mathX, laneB);
    }
    else if (bossPattern == PAT_SWEEP)
    {
        // Strafe edge to edge, raining a curtain of droppings with an egg every fourth drop
        const double target = (boss.mathX < (minX + maxX) / 2.0) ? (maxX - 26.0) : (minX + 26.0);
        const double dir = (target > boss.mathX) ? 1.0 : -1.0;
        const double spd = baseSpd * 0.55 * boss.def->travel * (rage ? 1.2 : 1.0);
        bossSweepVx = dir * spd;
        bossSweepTicks = std::max(20, static_cast<int>(std::ceil(std::abs(target - boss.mathX) / spd)));
        const int gap = 7;
        for (int n = 0; n * gap < bossSweepTicks; ++n)
        {
            BossShot s;
            s.delay = n * gap;
            s.straight = true;
            s.type = (n % 4 == 2) ? catchable() : EggType::BIRD_POOP;
            bossShots.push_back(s);
        }
    }
    else // PAT_RAIN
    {
        std::uniform_real_distribution<double> wide(-38.0, 38.0);
        std::uniform_real_distribution<double> near(-10.0, 10.0);
        for (int n = 0; n < 9; ++n)
        {
            BossShot s;
            s.delay = n * 5;
            s.straight = false;
            if (n == 2 || n == 6)
            {
                s.type = catchable();
                s.landX = clampX(basket.mathX + near(rng));
            }
            else
            {
                s.type = EggType::BIRD_POOP;
                s.landX = clampX(basket.mathX + wide(rng));
            }
            bossShots.push_back(s);
        }
    }

    const double modeGap = (mode == GameMode::EASY ? 1.25 : (mode == GameMode::MEDIUM ? 1.0 : 0.85));
    bossNextVolley = std::max(24, static_cast<int>(boss.def->volleyGap * modeGap * (rage ? 0.8 : 1.0)));
}

void GameEngine::spawnBossEgg(const BossShot &shot, const MyGrid &grid)
{
    Q_UNUSED(grid);
    const double mul = bossEggSpeedMul();
    constexpr double totalHeight = 50.0;

    FallingEgg e;
    e.active = true;
    e.type = shot.type;
    e.bossShot = true;
    e.telegraphed = (shot.type != EggType::BIRD_POOP); // Real egg is telegraphed
    e.mathX = boss.mathX + shot.offsetX;
    e.mathY = boss.mathY - BOSS_BELLY_DROP;
    e.vy = 0.12 * mul;
    e.gravity = (0.012 + totalHeight / 6500.0) * mul;
    e.maxSpeed = (1.5 + totalHeight / 70.0) * mul;
    if (!shot.straight)
    {
        const int fall = estimateFallTicks(e.mathY, mul);
        e.vx = std::clamp((shot.landX - e.mathX) / fall, -1.8, 1.8);
    }

    if (shot.type == EggType::BIRD_POOP)
    {
        switch (boss.def->trait)
        {
        case BossTrait::MUD:
        case BossTrait::INFERNO:
            e.leavesPatch = true;
            e.patchOnLand = PatchType::MUD;
            break;
        case BossTrait::FROST:
            e.leavesPatch = true;
            e.patchOnLand = PatchType::ICE;
            break;
        default:
            break; // GALE / NONE: ordinary droppings follow the normal difficulty rules
        }
    }

    fallingEggs.push_back(e);
    if (e.telegraphed)
    {
        spawnCatchParticles(e.mathX, e.mathY, QColor(255, 235, 59), 8);
    }
    else
    {
        spawnCatchParticles(e.mathX, e.mathY, boss.def->pal.glow, 4);
    }
}

void GameEngine::enterBossRage()
{
    boss.enraged = true;
    shake(26, 3);
    triggerSound(SoundId::BOSS_WARNING);
    bossNextVolley = std::min(bossNextVolley, 30);

    if (boss.def->trait == BossTrait::GALE || boss.def->trait == BossTrait::INFERNO)
    {
        bossGale = true;
    }

    const char *rageNote = "faster volleys, new attacks";
    switch (boss.def->trait)
    {
    case BossTrait::GALE:    rageNote = "howling crosswinds, new attacks"; break;
    case BossTrait::MUD:     rageNote = "mud bombs, new attacks"; break;
    case BossTrait::FROST:   rageNote = "freezing droppings, new attacks"; break;
    case BossTrait::INFERNO: rageNote = "firestorm winds, new attacks"; break;
    default: break;
    }
    addFloatingText(boss.mathX - 8.0, boss.mathY - BOSS_HALF_HEIGHT - 8.0, "💢 ENRAGED! 💢", QColor(255, 82, 82));
    statusMessage = QString("💢 %1 IS ENRAGED — %2!").arg(boss.def->name, rageNote);
}

void GameEngine::damageBoss(int dmg, double x, double y)
{
    if (bossPhase != BossPhase::FIGHT || boss.hp <= 0)
    {
        return;
    }

    boss.hp = std::max(0, boss.hp - dmg);
    boss.hurtFlash = 8;
    triggerSound(SoundId::BOSS_HIT);
    shake(dmg >= 3 ? 8 : 4, dmg >= 3 ? 3 : 1);

    // Sparks fly off the boss, and the damage number pops above the hit
    const QColor spark = boss.def->pal.crestLight;
    for (int i = 0; i < 10 + dmg * 3; ++i)
    {
        GameParticle p;
        p.x = boss.mathX + (static_cast<int>(rng() % 24) - 12);
        p.y = boss.mathY + (static_cast<int>(rng() % 14) - 4);
        p.vx = (static_cast<int>(rng() % 20) - 10) * 0.08;
        p.vy = 0.2 + (rng() % 10) * 0.06;
        p.color = (i % 2 == 0) ? spark : QColor(255, 255, 255);
        p.life = 14;
        p.maxLife = 14;
        particles.push_back(p);
    }
    addFloatingText(boss.mathX + 4.0, boss.mathY + BOSS_HALF_HEIGHT * 0.2,
                    QString("-%1 %2").arg(dmg).arg(dmg >= 3 ? "CRIT!" : ""), QColor(255, 235, 59));
    Q_UNUSED(x);
    Q_UNUSED(y);

    if (boss.hp <= 0)
    {
        defeatBoss();
    }
    else if (!boss.enraged && boss.hp * 100 <= boss.maxHp * 40)
    {
        enterBossRage();
    }
}

void GameEngine::updateBossFight(const MyGrid &grid)
{
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());
    const double span = std::max(50.0, static_cast<double>(maxX - minX));
    const double baseSpd = std::clamp(span / 70.0, 1.8, 5.5);

    static const int kFlap[4] = { 0, 1, 2, 1 };
    ++boss.animTick;
    boss.wingFrame = kFlap[(boss.animTick / (boss.enraged ? 3 : 4)) % 4];
    if (boss.hurtFlash > 0)
    {
        --boss.hurtFlash;
    }
    boss.mathY = bossHoverY + 1.6 * std::sin(boss.animTick * 0.09);

    // Queued eggs leave the boss when their delay runs out
    for (size_t i = 0; i < bossShots.size(); )
    {
        if (--bossShots[i].delay <= 0)
        {
            spawnBossEgg(bossShots[i], grid);
            bossShots.erase(bossShots.begin() + static_cast<long>(i));
        }
        else
        {
            ++i;
        }
    }

    // Movement: strafe during a sweep, otherwise glide to wherever the next volley is aimed
    const double lo = minX + 26.0;
    const double hi = maxX - 26.0;
    if (bossSweepTicks > 0)
    {
        boss.mathX += bossSweepVx;
        --bossSweepTicks;
    }
    else
    {
        const double repo = baseSpd * 1.8 * boss.def->travel * (boss.enraged ? 1.2 : 1.0);
        boss.mathX += std::clamp(bossTargetX - boss.mathX, -repo, repo);
    }
    if (lo < hi)
    {
        boss.mathX = std::clamp(boss.mathX, lo, hi);
    }

    // Charge-up glow sparks gather at the belly
    if (boss.chargeTicks > 0 && boss.animTick % 3 == 0)
    {
        GameParticle p;
        p.x = boss.mathX + (static_cast<int>(rng() % 14) - 7);
        p.y = boss.mathY - BOSS_BELLY_DROP + 2 + static_cast<int>(rng() % 4);
        p.vx = -(p.x - boss.mathX) * 0.04;
        p.vy = 0.15;
        p.color = boss.def->pal.glow;
        p.life = 10;
        p.maxLife = 10;
        particles.push_back(p);
    }

    if (boss.chargeTicks > 0)
    {
        if (--boss.chargeTicks <= 0)
        {
            fireBossVolley(grid);
        }
    }
    else if (bossShots.empty() && bossSweepTicks <= 0)
    {
        if (--bossNextVolley <= 0)
        {
            beginBossCharge(grid);
        }
    }

    // Rage ember trail
    if (boss.enraged && boss.animTick % 4 == 0)
    {
        GameParticle p;
        p.x = boss.mathX + (static_cast<int>(rng() % 40) - 20);
        p.y = boss.mathY + (static_cast<int>(rng() % 20) - 6);
        p.vx = 0.0;
        p.vy = 0.12;
        p.color = QColor(255, 82, 82, 220);
        p.life = 12;
        p.maxLife = 12;
        particles.push_back(p);
    }
}

void GameEngine::defeatBoss()
{
    bossPhase = BossPhase::DEFEATED;
    bossPhaseTicks = 0;
    boss.defeatTicks = 1;
    boss.chargeTicks = 0;
    bossShots.clear();
    bossSweepTicks = 0;
    bossGale = false;
    bossRewardDropped = false;
    highestBossBeaten = std::max(highestBossBeaten, level);
    ++bossesDefeated;

    // Any eggs still in the air vanish in a puff: nothing can cost a heart after the win.
    // (Marked inactive so an in-progress egg-update loop cleans them up safely.)
    for (auto &egg : fallingEggs)
    {
        if (egg.active)
        {
            spawnCatchParticles(egg.mathX, egg.mathY, QColor(255, 255, 255), 6);
            egg.active = false;
        }
    }

    const int bonus = 150 + 50 * boss.tier;
    score += bonus;
    highScore = std::max(highScore, score);

    shake(34, 4);
    triggerSound(SoundId::BOSS_DEFEAT);
    addFloatingText(boss.mathX - 22.0, boss.mathY - 22.0, QString("🏆 %1 DEFEATED! +%2").arg(boss.def->name).arg(bonus), QColor(255, 214, 10));
    statusMessage = QString("🏆 %1 DEFEATED! +%2 pts — a trophy egg is falling!").arg(boss.def->name).arg(bonus);
    bossBannerTicks = 90;
}

void GameEngine::updateBossDefeat(const MyGrid &grid)
{
    const int maxY = grid.screenToMathY(0);
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());

    ++bossPhaseTicks;
    ++boss.animTick;
    ++boss.defeatTicks;
    boss.wingFrame = (boss.animTick / 6) % 3;

    if (bossPhaseTicks < 64)
    {
        // Convulsing and bursting apart
        boss.mathX += ((bossPhaseTicks % 4 < 2) ? 0.5 : -0.5);
        if (bossPhaseTicks % 5 == 0)
        {
            spawnExplosionParticles(boss.mathX + (static_cast<int>(rng() % 30) - 15),
                                    boss.mathY + (static_cast<int>(rng() % 22) - 8));
            if (bossPhaseTicks % 10 == 0)
            {
                shake(4, 2);
            }
        }
    }

    // The trophy egg drops gently over the player
    if (bossPhaseTicks == 50 && !bossRewardDropped)
    {
        bossRewardDropped = true;
        FallingEgg e;
        e.active = true;
        e.type = EggType::BOSS_REWARD;
        e.mathX = std::clamp(basket.mathX + (static_cast<int>(rng() % 11) - 5),
                             static_cast<double>(minX + 6), static_cast<double>(maxX - 6));
        e.mathY = boss.mathY - 8.0;
        e.vy = 0.05;
        e.gravity = 0.007;
        e.maxSpeed = 0.9;
        fallingEggs.push_back(e);
        addFloatingText(e.mathX, e.mathY, "👑 TROPHY EGG!", QColor(255, 214, 10));
    }

    if (bossPhaseTicks >= 70)
    {
        boss.mathY += 0.5 + (bossPhaseTicks - 70) * 0.05;
    }

    if (boss.mathY > maxY + BOSS_HALF_HEIGHT + 20 || bossPhaseTicks > 260)
    {
        boss.present = false;
        boss.defeatTicks = 0;
        bossPhase = BossPhase::NONE;
        eggSpawnCooldown = std::max(eggSpawnCooldown, 80);
        statusMessage = "The skies clear. The flock returns!";
        // Fight points stay in the score, but level progress restarts cleanly at the next level
        if (level < MAX_LEVEL)
        {
            levelCredit = score - levelStartScore(level + 1);
        }
        checkLevelUp(grid);
    }
}

// =============================================================
// GAMEPLAY WEATHER: wind, rain, night fog
// =============================================================

double GameEngine::getNightFactor() const
{
    const double p = getDayNightProgress();
    const double nr = getNightDurationRatio();
    const double nightStart = 1.0 - nr;
    const double duskStart = nightStart - 0.07;
    auto smooth = [](double t) {
        t = std::clamp(t, 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    };
    if (p >= nightStart)
    {
        const double t = (p - nightStart) / nr;
        return t <= 0.78 ? 1.0 : smooth((1.0 - t) / 0.22);
    }
    if (p >= duskStart)
    {
        return smooth((p - duskStart) / (nightStart - duskStart));
    }
    return 0.0;
}

void GameEngine::updateWeatherEffects(const MyGrid &grid)
{
    updateWind();
    updateRainAndFog();
    updateHazardSpawns(grid);

    // Wind-blown leaves and dust make the invisible visible
    if (std::abs(windStrength) > 0.15 && dayNightTick % 2 == 0)
    {
        static const QColor leaf[] = { QColor(139, 195, 74), QColor(104, 159, 56), QColor(188, 143, 64),
                                       QColor(255, 235, 160), QColor(205, 220, 140) };
        const int minX = grid.screenToMathX(0);
        const int maxX = grid.screenToMathX(grid.getWidth());
        GameParticle p;
        p.x = windStrength > 0 ? minX - 1.0 : maxX + 1.0;
        p.y = groundMathY + 18 + static_cast<int>(rng() % 55);
        p.vx = windStrength * (1.7 + (rng() % 10) * 0.05);
        p.vy = 0.12 + (rng() % 10) * 0.03;
        p.color = leaf[rng() % 5];
        p.life = 34;
        p.maxLife = 34;
        particles.push_back(p);
    }
}

void GameEngine::updateWind()
{
    const bool gale = bossGale && bossPhase == BossPhase::FIGHT;
    const int gate = (mode == GameMode::EASY) ? 3 : (mode == GameMode::MEDIUM ? 2 : 1);
    const double maxStrength = (mode == GameMode::EASY) ? 0.55 : (mode == GameMode::MEDIUM ? 0.8 : 1.0);

    if (gale)
    {
        // Relentless alternating crosswind while a storm-boss rages
        windForced = true;
        windPhase = WindPhase::GUST;
        windTarget = (((dayNightTick / 170) % 2 == 0) ? 1.0 : -1.0) * 0.72;
        windIncomingDir = windTarget > 0 ? 1.0 : -1.0;
    }
    else if (thunderstormActive)
    {
        windForced = true;
        windPhase = WindPhase::GUST;
        windTarget = 0.5 * std::sin(thunderstormTicks * 0.045);
    }
    else if (windForced)
    {
        // Forced wind just ended: settle back into the normal rhythm
        windForced = false;
        windPhase = WindPhase::CALM;
        windPhaseTicks = 360;
        windTarget = 0.0;
    }
    else if (level < gate)
    {
        windPhase = WindPhase::CALM;
        windTarget = 0.0;
    }
    else
    {
        if (windPhaseTicks > 0)
        {
            --windPhaseTicks;
        }

        if (windPhase == WindPhase::CALM && windPhaseTicks <= 0)
        {
            windIncomingDir = (rng() % 2 == 0) ? 1.0 : -1.0;
            windGustStrength = (0.55 + (rng() % 46) / 100.0) * maxStrength;
            windPhase = WindPhase::WARNING;
            windPhaseTicks = 40; // ~1.2s of warning so players can brace
            windTarget = 0.0;
            addFloatingText(basket.mathX, groundMathY + 34.0,
                            windIncomingDir > 0 ? "💨 GUST INCOMING ▶▶" : "◀◀ GUST INCOMING 💨", QColor(190, 225, 255));
            statusMessage = windIncomingDir > 0 ? "💨 Gust incoming from the left — eggs will drift RIGHT!"
                                                : "💨 Gust incoming from the right — eggs will drift LEFT!";
        }
        else if (windPhase == WindPhase::WARNING && windPhaseTicks <= 0)
        {
            windPhase = WindPhase::GUST;
            windPhaseTicks = 170 + static_cast<int>(rng() % 140);
            windTarget = windIncomingDir * windGustStrength;
            triggerSound(SoundId::WIND_GUST);
        }
        else if (windPhase == WindPhase::GUST && windPhaseTicks <= 0)
        {
            windPhase = WindPhase::CALM;
            const double modeCalm = (mode == GameMode::EASY ? 1.3 : (mode == GameMode::HARD ? 0.8 : 1.0));
            windPhaseTicks = static_cast<int>((520 + rng() % 420) * modeCalm * std::max(0.65, 1.0 - 0.01 * level));
            windTarget = 0.0;
            statusMessage = "The wind dies down.";
        }
    }

    windStrength += (windTarget - windStrength) * 0.045;
    if (windTarget == 0.0 && std::abs(windStrength) < 0.003)
    {
        windStrength = 0.0;
    }
}

void GameEngine::updateRainAndFog()
{
    // ---- Rain showers (thunderstorms bring their own rain) ----
    const int rainGate = (mode == GameMode::EASY) ? 4 : (mode == GameMode::MEDIUM ? 3 : 2);
    if (rainActive)
    {
        if (--rainTicks <= 0 || thunderstormActive)
        {
            rainActive = false;
            rainCooldown = 1100 + static_cast<int>(rng() % 1200);
            statusMessage = "The rain eases. The ground will dry out soon.";
        }
    }
    else if (level >= rainGate && !thunderstormActive && bossPhase == BossPhase::NONE)
    {
        if (--rainCooldown <= 0)
        {
            rainCooldown = 900 + static_cast<int>(rng() % 900);
            const int chance = (mode == GameMode::EASY) ? 45 : (mode == GameMode::MEDIUM ? 65 : 80);
            if (static_cast<int>(rng() % 100) < chance)
            {
                rainActive = true;
                rainTicks = 520 + static_cast<int>(rng() % 260);
                addFloatingText(basket.mathX, groundMathY + 34.0, "🌧 RAIN: SLIPPERY GROUND!", QColor(150, 195, 240));
                statusMessage = "🌧 Rain shower! The ground is slick — your basket will slide, and mud may form.";
            }
        }
    }

    const double wetTarget = thunderstormActive ? 1.0 : (rainActive ? 0.8 : 0.0);
    if (wetness < wetTarget)
    {
        wetness = std::min(wetTarget, wetness + 0.012);
    }
    else
    {
        wetness = std::max(wetTarget, wetness - 0.005); // ~5s to dry out
    }

    // ---- Night fog: thick above, clear near the ground; hides eggs until they drop into view ----
    const int fogGate = (mode == GameMode::EASY) ? 5 : (mode == GameMode::MEDIUM ? 4 : 3);
    const double fogMax = (mode == GameMode::EASY) ? 0.50 : (mode == GameMode::MEDIUM ? 0.65 : 0.80);
    double fogTarget = 0.0;
    if (level >= fogGate && bossPhase == BossPhase::NONE && !thunderstormActive)
    {
        fogTarget = fogMax * getNightFactor();
    }
    fogDensity += (fogTarget - fogDensity) * 0.012;
    if (fogTarget == 0.0 && fogDensity < 0.01)
    {
        fogDensity = 0.0;
    }

    if (fogTarget > 0.3 && !fogAnnounced)
    {
        fogAnnounced = true;
        addFloatingText(basket.mathX, groundMathY + 34.0, "🌫 NIGHT FOG ROLLING IN", QColor(200, 210, 225));
        statusMessage = "🌫 Night fog rolls in! Eggs are hard to see up high — watch the fog line.";
    }
    else if (fogTarget < 0.05)
    {
        fogAnnounced = false;
    }
}

// =============================================================
// GROUND HAZARDS: slime (original), mud (rain), ice (night frost)
// =============================================================

void GameEngine::spawnPatch(PatchType type, double x, double radius, int life)
{
    // Don't stack patches on top of each other
    for (const auto &pt : stickyPuddles)
    {
        if (std::abs(pt.mathX - x) < pt.radius + radius + 2.0)
        {
            return;
        }
    }
    if (stickyPuddles.size() >= 9)
    {
        stickyPuddles.erase(stickyPuddles.begin());
    }

    StickyPuddle sp;
    sp.mathX = x;
    sp.radius = radius;
    sp.lifeTicks = life;
    sp.maxLifeTicks = life;
    sp.type = type;
    sp.ageTicks = 0;
    stickyPuddles.push_back(sp);

    if (type == PatchType::ICE)
    {
        triggerSound(SoundId::FROST_FORM);
        addFloatingText(x, groundMathY + 6.0, "❄ ICE! (slippery)", QColor(150, 235, 255));
    }
    else if (type == PatchType::MUD)
    {
        triggerSound(SoundId::POOP_GROUND);
        addFloatingText(x, groundMathY + 6.0, "MUD! (slow)", QColor(188, 143, 100));
    }
}

GameEngine::PatchEffect GameEngine::updatePatches(const MyGrid &grid)
{
    Q_UNUSED(grid);
    PatchEffect fx;
    bool onSlime = false, onMud = false, onIce = false;

    for (size_t i = 0; i < stickyPuddles.size(); )
    {
        StickyPuddle &pt = stickyPuddles[i];
        pt.lifeTicks--;
        pt.ageTicks++;
        if (pt.lifeTicks <= 0)
        {
            stickyPuddles[i] = stickyPuddles.back();
            stickyPuddles.pop_back();
            continue;
        }

        // Forming mud/ice is a visible warning only: it doesn't bite until it has formed
        if (!pt.isForming())
        {
            const double dist = std::abs(basket.mathX - pt.mathX);
            if (dist <= basket.halfWidth + pt.currentRadius())
            {
                switch (pt.type)
                {
                case PatchType::SLIME: onSlime = true; fx.speedMul = std::min(fx.speedMul, 0.60); break;
                case PatchType::MUD:   onMud = true;   fx.speedMul = std::min(fx.speedMul, 0.30); break;
                case PatchType::ICE:   onIce = true;   fx.slipAlpha = std::min(fx.slipAlpha, 0.20); break;
                }
            }
        }
        ++i;
    }

    basketOnPatch = onSlime || onMud || onIce;
    basketPatchType = onIce ? PatchType::ICE : (onMud ? PatchType::MUD : PatchType::SLIME);

    if (onSlime && !onMud && !onIce && dayNightTick % 6 == 0)
    {
        GameParticle p;
        p.x = basket.mathX + ((rng() % 10) - 5);
        p.y = basket.mathY + 1;
        p.vx = ((rng() % 6) - 3) * 0.05;
        p.vy = 0.15;
        p.color = QColor(109, 76, 65, 200);
        p.life = 10;
        p.maxLife = 10;
        particles.push_back(p);
    }
    if (onMud && dayNightTick % 3 == 0)
    {
        GameParticle p;
        p.x = basket.mathX + ((rng() % 14) - 7);
        p.y = basket.mathY + 1;
        p.vx = ((rng() % 8) - 4) * 0.07;
        p.vy = 0.25 + (rng() % 5) * 0.05;
        p.color = (rng() % 2 == 0) ? QColor(121, 85, 58) : QColor(78, 52, 38);
        p.life = 11;
        p.maxLife = 11;
        particles.push_back(p);
    }
    if (onIce && dayNightTick % 3 == 0)
    {
        GameParticle p;
        p.x = basket.mathX + ((rng() % 14) - 7);
        p.y = basket.mathY + 1;
        p.vx = ((rng() % 8) - 4) * 0.05;
        p.vy = 0.30 + (rng() % 5) * 0.05;
        p.color = (rng() % 2 == 0) ? QColor(214, 245, 255) : QColor(120, 220, 255);
        p.life = 11;
        p.maxLife = 11;
        particles.push_back(p);
    }
    return fx;
}

void GameEngine::updateHazardSpawns(const MyGrid &grid)
{
    const int minX = grid.screenToMathX(0);
    const int maxX = grid.screenToMathX(grid.getWidth());
    const double nf = getNightFactor();

    int mudCount = 0, iceCount = 0;
    for (const auto &pt : stickyPuddles)
    {
        mudCount += (pt.type == PatchType::MUD) ? 1 : 0;
        iceCount += (pt.type == PatchType::ICE) ? 1 : 0;
    }

    auto pickX = [&](bool nearPlayer) {
        if (nearPlayer)
        {
            const double dir = (rng() % 2 == 0) ? 1.0 : -1.0;
            return std::clamp(basket.mathX + dir * (16.0 + static_cast<double>(rng() % 28)),
                              static_cast<double>(minX + 10), static_cast<double>(maxX - 10));
        }
        return static_cast<double>(minX + 10 + static_cast<int>(rng() % std::max(1, maxX - minX - 20)));
    };

    // Natural spawns only happen in calm play: boss fights get their hazards from the boss itself
    if (bossPhase == BossPhase::NONE)
    {
        // Rain softens the ground into mud
        if (wetness > 0.55 && level >= 3)
        {
            if (--mudCooldown <= 0)
            {
                const double modeSlow = (mode == GameMode::EASY ? 1.5 : (mode == GameMode::HARD ? 0.8 : 1.0));
                mudCooldown = static_cast<int>((260 + rng() % 200) * modeSlow);
                if (mudCount < 3)
                {
                    spawnPatch(PatchType::MUD, pickX(rng() % 100 < 55), 3.5 + (rng() % 25) / 10.0, 560 + static_cast<int>(rng() % 200));
                }
            }
        }

        // Deep night freezes patches of ground
        const int frostGate = (mode == GameMode::EASY) ? 6 : (mode == GameMode::MEDIUM ? 4 : 3);
        if (nf > 0.6 && level >= frostGate && !thunderstormActive)
        {
            if (--frostCooldown <= 0)
            {
                const double modeSlow = (mode == GameMode::EASY ? 1.4 : (mode == GameMode::HARD ? 0.85 : 1.0));
                frostCooldown = static_cast<int>((230 + rng() % 190) * modeSlow);
                if (iceCount < 3)
                {
                    spawnPatch(PatchType::ICE, pickX(rng() % 100 < 50), 4.5 + (rng() % 25) / 10.0, 700 + static_cast<int>(rng() % 200));
                }
            }
        }
    }

    // Sunrise melts the frost (patches shrink away over their melt window)
    if (nf < 0.3)
    {
        for (auto &pt : stickyPuddles)
        {
            if (pt.type == PatchType::ICE && pt.lifeTicks > StickyPuddle::MELT_TICKS)
            {
                pt.lifeTicks = StickyPuddle::MELT_TICKS;
            }
        }
    }
}
