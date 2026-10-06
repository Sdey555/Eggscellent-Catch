#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPixmap>
#include <QPainter>
#include <QFont>
#include <QResizeEvent>
#include <QKeyEvent>
#include <algorithm>
#include <array>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Eggscellent Catch");

    scale = ui->spinScale->value();
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    myGrid.setScale(scale);

    ui->frame->installEventFilter(this);
    this->installEventFilter(this);

    connect(ui->frame, &my_label::panDelta, this, &MainWindow::onGridPanned);
    connect(ui->frame, &my_label::sendMousePosition, this, &MainWindow::onMouseMoved);
    connect(ui->frame, &my_label::Mouse_Pos, this, &MainWindow::onMouseLeftClicked);
    connect(ui->frame, &my_label::rightClicked, this, &MainWindow::onMouseRightClicked);

    // Level indicator in the top ribbon (placed right after BEST score)
    lblLevel = new QLabel("LEVEL 1", this);
    lblLevel->setObjectName("lblLevel");
    lblLevel->setStyleSheet(
        "color: #c77dff; font-size: 14px; font-weight: bold;"
        "background-color: #22172e; border: 1px solid #5a3d7a;"
        "border-radius: 4px; padding: 3px 8px;");
    const int bestIdx = ui->ribbonLayout->indexOf(ui->lblHighScore);
    ui->ribbonLayout->insertWidget(bestIdx + 1, lblLevel);

    ui->lblHelp->setText(
        "Controls: Mouse or [A / D] / [◄ / ►] Move Basket | Right-Click or [Space] Pause | "
        "Left-Click Resume/Restart | [R] Restart | 🥚 +10 | ⭐ +50 | 💣 Game Over | "
        "Level up every 100 pts: faster eggs, smaller basket!");

    rng.seed(std::random_device{}());

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoopTick);
    gameTimer->start(30);

    initGame();
    updateHUD();
}

MainWindow::~MainWindow()
{
    if (gameTimer)
    {
        gameTimer->stop();
    }
    delete ui;
}

void MainWindow::initGame()
{
    score = 0;
    hearts = 3;
    gameState = GameState::PLAYING;
    gameOverReason.clear();

    keyLeftPressed = false;
    keyRightPressed = false;

    fallingEgg.active = false;
    fallingEgg.animTick = 0;
    fallingEgg.vy = 0.08;
    fallingEgg.gravity = 0.015;
    fallingEgg.maxSpeed = 1.6;

    particles.clear();
    floatingTexts.clear();

    const int width = std::max(ui->frame->width(), 640);
    const int height = std::max(ui->frame->height(), 480);
    myGrid.setDimensions(width, height);
    myGrid.setScale(scale);

    const int minMathX = myGrid.screenToMathX(0);
    const int maxMathX = myGrid.screenToMathX(width);
    const int minMathY = myGrid.screenToMathY(height);
    const int maxMathY = myGrid.screenToMathY(0);

    groundMathY = minMathY + 9;

    const double screenSpan = std::max(50.0, static_cast<double>(maxMathX - minMathX));

    // Initialize Basket (width depends on level)
    level = 1;
    levelBannerTicks = 0;
    applyLevelSettings();
    basket.mathX = (minMathX + maxMathX) / 2.0;
    basket.mathY = groundMathY + 1;
    basket.height = 5;
    basket.speed = std::clamp(screenSpan / 70.0, 1.6, 5.2);

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

    // Initial egg drop cooldown (~3.0 seconds)
    eggSpawnCooldown = 90;

    ui->lblStatus->setText("Level 1! Move the basket with your MOUSE or [A/D] / arrow keys!");
}

void MainWindow::resetGame()
{
    initGame();
    ui->btnPause->setText("Pause");
    updateHUD();
    redrawPixels();
}

void MainWindow::togglePause()
{
    if (gameState == GameState::GAME_OVER)
    {
        resetGame();
    }
    else if (gameState == GameState::PLAYING)
    {
        gameState = GameState::PAUSED;
        ui->btnPause->setText("Resume");
        ui->lblStatus->setText("Game Paused. Press Space / Right-Click / Left-Click to Resume.");
    }
    else if (gameState == GameState::PAUSED)
    {
        gameState = GameState::PLAYING;
        ui->btnPause->setText("Pause");
        ui->lblStatus->setText("Game Resumed!");
    }
}

// -------------------------------------------------------------
// Levels / Difficulty
// -------------------------------------------------------------
int MainWindow::levelForScore(int s) const
{
    return std::min(MAX_LEVEL, 1 + s / POINTS_PER_LEVEL);
}

double MainWindow::levelSpeedMultiplier() const
{
    // Level 1 = 1.0x, Level 10 = 2.8x
    return 1.0 + 0.20 * (level - 1);
}

void MainWindow::applyLevelSettings()
{
    // Basket shrinks every level: 6 -> 5 -> 4 -> 3 -> 2 (half-width in raster pixels)
    basket.halfWidth = std::max(2, 6 - (level - 1));
}

void MainWindow::checkLevelUp()
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
    ui->lblStatus->setText(QString("🚀 LEVEL %1! Eggs fall faster%2")
                               .arg(level)
                               .arg(shrank ? " and your basket got smaller!" : "!"));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    redrawPixels();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        keyPressEvent(ke);
        return true;
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        keyReleaseEvent(ke);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    const int key = event->key();

    if (key == Qt::Key_Left || key == Qt::Key_A)
    {
        keyLeftPressed = true;
        mouseControl = false; // keyboard takes over until the mouse moves again
    }
    else if (key == Qt::Key_Right || key == Qt::Key_D)
    {
        keyRightPressed = true;
        mouseControl = false;
    }
    else if (key == Qt::Key_Space)
    {
        togglePause();
    }
    else if (key == Qt::Key_R)
    {
        resetGame();
    }
    else
    {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
    {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    const int key = event->key();

    if (key == Qt::Key_Left || key == Qt::Key_A)
    {
        keyLeftPressed = false;
    }
    else if (key == Qt::Key_Right || key == Qt::Key_D)
    {
        keyRightPressed = false;
    }
    else
    {
        QMainWindow::keyReleaseEvent(event);
    }
}

void MainWindow::gameLoopTick()
{
    if (gameState == GameState::PLAYING)
    {
        updatePhysics();
        hudTick++;
    }
    redrawPixels();
}

void MainWindow::updatePhysics()
{
    const int frameW = ui->frame->width();
    const int frameH = ui->frame->height();
    const int minMathX = myGrid.screenToMathX(0);
    const int maxMathX = myGrid.screenToMathX(frameW);
    const int minMathY = myGrid.screenToMathY(frameH);

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

    // Mouse control: glide towards the cursor (capped speed keeps it fair, not instant teleport)
    if (mouseControl && !keyLeftPressed && !keyRightPressed)
    {
        const double targetX = static_cast<double>(mouseScreenX - myGrid.getOriginX()) / scale;
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
    updateBirds();
    updateEggSpawn();

    // 3. Update Falling Egg
    updateFallingEgg();

    // 4. Update Particle Effects
    updateParticles();
}

void MainWindow::updateBirds()
{
    const int frameW = ui->frame->width();
    const int minMathX = myGrid.screenToMathX(0);
    const int maxMathX = myGrid.screenToMathX(frameW);
    const int maxMathY = myGrid.screenToMathY(0);

    for (size_t i = 0; i < birds.size(); ++i)
    {
        Bird &bird = birds[i];

        // Maintain comfortable flying height
        const int targetY = maxMathY - 9 - static_cast<int>(i * 4);
        bird.mathY = targetY;

        // Wing flapping cycle
        bird.wingTick++;
        if (bird.wingTick >= 6)
        {
            bird.wingTick = 0;
            bird.wingFrame = 1 - bird.wingFrame;
        }

        if (bird.isLaying)
        {
            // Flutter in place while warning player
            bird.layingCountdown--;
            bird.mathX += (bird.layingCountdown % 2 == 0 ? 0.25 : -0.25);

            if (bird.layingCountdown <= 0)
            {
                // Egg is laid!
                fallingEgg.active = true;
                fallingEgg.mathX = bird.mathX;
                fallingEgg.mathY = bird.mathY - 2.0;
                fallingEgg.type = bird.pendingEggType;
                fallingEgg.animTick = 0;
                // Initial downward velocity
                fallingEgg.vy = 0.08;
                // Velocity increment for gravity effect per tick (scaled by level)
                const int totalHeight = std::max(30, maxMathY - groundMathY);
                const double speedMul = levelSpeedMultiplier();
                fallingEgg.gravity = (0.010 + (totalHeight / 7500.0)) * speedMul;
                fallingEgg.maxSpeed = (1.2 + (totalHeight / 80.0)) * speedMul;

                bird.isLaying = false;

                if (fallingEgg.type == EggType::GOLDEN)
                {
                    ui->lblStatus->setText("✨ A bird laid a SPECIAL GOLDEN EGG! Catch it for bonus points!");
                }
                else if (fallingEgg.type == EggType::BOMB)
                {
                    ui->lblStatus->setText("⚠️ A bird dropped a BOMB! Do NOT catch it in your basket!");
                }
                else
                {
                    ui->lblStatus->setText("An egg is falling! Catch it!");
                }
            }
        }
        else
        {
            bird.mathX += bird.direction * bird.speed * (1.0 + 0.15 * (level - 1));

            // Turn around at sky boundary
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

void MainWindow::updateEggSpawn()
{
    // Requirements: "birds are laying eggs (one at a time with variable but long enough duration)"
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
            const int frameW = ui->frame->width();
            const int minMathX = myGrid.screenToMathX(0);
            const int maxMathX = myGrid.screenToMathX(frameW);

            // Find birds that are safely within the visible window
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

                // Egg type distribution (level 1): Regular 65%, Golden 18%, Bomb 17%
                // Each level adds +4% bombs (capped at 50%), taken from regular eggs
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
                // Warning time shrinks with level: ~0.85s at L1 down to extremely short 8 ticks
                chosenBird.layingCountdown = std::max(8, 28 - 3 * (level - 1));
                chosenBird.pendingEggType = eType;

                // Variable delay before next egg (3.0s-5.0s at L1, shrinking much faster with level)
                const double gapFactor = std::max(0.25, 1.0 - 0.09 * (level - 1));
                std::uniform_int_distribution<int> cooldownDist(static_cast<int>(95 * gapFactor),
                                                                static_cast<int>(160 * gapFactor));
                eggSpawnCooldown = cooldownDist(rng);
            }
        }
    }
}

void MainWindow::updateFallingEgg()
{
    if (!fallingEgg.active)
    {
        return;
    }

    // Velocity increment for gravity effect
    const double prevEggY = fallingEgg.mathY;
    fallingEgg.vy = std::min(fallingEgg.maxSpeed, fallingEgg.vy + fallingEgg.gravity);
    fallingEgg.mathY -= fallingEgg.vy;
    fallingEgg.animTick++;

    // 1. Collision detection with Basket Rim
    // (swept test: fast eggs on high levels may jump past the rim in a single tick)
    const double rimY = basket.mathY + basket.height;
    if (fallingEgg.mathY <= rimY + 0.6 && prevEggY >= basket.mathY)
    {
        const double deltaX = std::abs(fallingEgg.mathX - basket.mathX);
        if (deltaX <= basket.halfWidth + 0.8)
        {
            // Egg caught!
            fallingEgg.active = false;

            if (fallingEgg.type == EggType::REGULAR)
            {
                score += 10;
                highScore = std::max(highScore, score);
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(255, 255, 230), 12);
                addFloatingText(basket.mathX, rimY + 3.0, "+10", QColor(46, 204, 113));
                ui->lblStatus->setText("Caught Regular Egg! +10 pts");
            }
            else if (fallingEgg.type == EggType::GOLDEN)
            {
                score += 50;
                highScore = std::max(highScore, score);
                spawnCatchParticles(fallingEgg.mathX, rimY, QColor(255, 215, 0), 24);
                addFloatingText(basket.mathX, rimY + 3.0, "⭐ +50 BONUS! ⭐", QColor(255, 215, 0));
                ui->lblStatus->setText("⭐ BONUS! Caught Golden Egg! +50 pts! ⭐");
            }
            else if (fallingEgg.type == EggType::BOMB)
            {
                // Catching a bomb ends game instantly
                spawnExplosionParticles(fallingEgg.mathX, rimY);
                addFloatingText(basket.mathX, rimY + 4.0, "💥 BOOM! 💥", QColor(231, 76, 60));
                gameOverReason = "Caught a Bomb! Instant Game Over!";
                gameState = GameState::GAME_OVER;
                ui->lblStatus->setText("💥 KABOOM! You caught a bomb! Game Over!");
            }

            if (gameState == GameState::PLAYING)
            {
                checkLevelUp();
            }
            updateHUD();
            return;
        }
    }

    // 2. Collision with Ground
    if (fallingEgg.mathY <= groundMathY + 1.0)
    {
        fallingEgg.active = false;

        if (fallingEgg.type == EggType::REGULAR || fallingEgg.type == EggType::GOLDEN)
        {
            // Miss penalty: hearts decreased by 1
            spawnSplatParticles(fallingEgg.mathX, groundMathY + 1, fallingEgg.type);
            hearts--;
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "SPLAT! -1 ❤", QColor(231, 76, 60));

            if (hearts <= 0)
            {
                hearts = 0;
                gameOverReason = "Out of Hearts! 3 misses reached.";
                gameState = GameState::GAME_OVER;
                ui->lblStatus->setText("Game Over! You lost all 3 hearts.");
            }
            else
            {
                ui->lblStatus->setText(QString("Egg missed and cracked! Lost 1 heart (%1 left).").arg(hearts));
            }
        }
        else if (fallingEgg.type == EggType::BOMB)
        {
            // Bomb safely detonates on ground: player successfully avoided it!
            spawnCatchParticles(fallingEgg.mathX, groundMathY + 1, QColor(120, 120, 120), 10);
            addFloatingText(fallingEgg.mathX, groundMathY + 3.0, "DODGED BOMB! Safe!", QColor(168, 199, 250));
            ui->lblStatus->setText("Bomb safely exploded on the ground! Nice dodge!");
        }

        updateHUD();
    }
}

void MainWindow::updateParticles()
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

// -------------------------------------------------------------
// Raster Grid & Raster Pixels Rendering
// -------------------------------------------------------------
void MainWindow::paintCell(QPainter &painter, int mathX, int mathY, const QColor &color)
{
    const int sx = myGrid.mathToScreenX(mathX);
    const int sy = myGrid.mathToScreenY(mathY);
    if (sx + scale < 0 || sx >= ui->frame->width() || sy + scale < 0 || sy >= ui->frame->height())
    {
        return;
    }
    // 1. Solid raster cell without any grid-line gaps
    painter.fillRect(sx, sy, scale, scale, color);

    // 2. Inner phosphor glow highlight for vibrant arcade pixel appearance
    if (scale >= 4)
    {
        const int inset = (scale >= 8 ? 2 : 1);
        const int innerSize = scale - 2 * inset;
        if (innerSize >= 1)
        {
            QColor centerColor = color.lighter(130);
            painter.fillRect(sx + inset, sy + inset, innerSize, innerSize, centerColor);
        }
    }
}

void MainWindow::rasterizeGround(int minX, int maxX, int gY, int minY)
{
    for (int x = minX; x <= maxX; ++x)
    {
        // Top lush grass
        myPixels.setPixel(x, gY, ((x % 3 == 0) ? QColor(88, 214, 141) : QColor(46, 204, 113)));
        // Sub-grass
        myPixels.setPixel(x, gY - 1, QColor(39, 174, 96));

        // Earth layers
        for (int y = gY - 2; y >= minY; --y)
        {
            if ((x * 7 + y * 13) % 11 == 0)
            {
                myPixels.setPixel(x, y, QColor(121, 85, 72)); // soil speckle
            }
            else if (y > gY - 5)
            {
                myPixels.setPixel(x, y, QColor(93, 64, 55));
            }
            else
            {
                myPixels.setPixel(x, y, QColor(78, 52, 46));
            }
        }
    }
}

void MainWindow::rasterizeBasket(const Basket &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = b.mathY;
    const int hw = b.halfWidth;

    // Handles
    myPixels.setPixel(bx - hw, by + 4, QColor(93, 64, 55));
    myPixels.setPixel(bx + hw, by + 4, QColor(93, 64, 55));

    // Top Rim
    myPixels.setPixel(bx - hw, by + 3, QColor(109, 76, 65));
    for (int x = bx - hw + 1; x <= bx + hw - 1; ++x)
    {
        myPixels.setPixel(x, by + 3, QColor(215, 204, 200)); // rim highlight
    }
    myPixels.setPixel(bx + hw, by + 3, QColor(109, 76, 65));

    // Wicker body rows with woven pattern
    for (int y = by + 2; y >= by + 1; --y)
    {
        myPixels.setPixel(bx - hw + 1, y, QColor(93, 64, 55));
        for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
        {
            if ((x + y) % 2 == 0)
            {
                myPixels.setPixel(x, y, QColor(161, 136, 127));
            }
            else
            {
                myPixels.setPixel(x, y, QColor(141, 110, 99));
            }
        }
        myPixels.setPixel(bx + hw - 1, y, QColor(93, 64, 55));
    }

    // Base
    for (int x = bx - hw + 2; x <= bx + hw - 2; ++x)
    {
        myPixels.setPixel(x, by, QColor(93, 64, 55));
    }
}

void MainWindow::rasterizeBird(const Bird &b)
{
    const int bx = static_cast<int>(std::round(b.mathX));
    const int by = static_cast<int>(std::round(b.mathY));
    const int dir = b.direction;

    // Relative bird offsets (facing right):
    // Body layout: 8x5 raster pixels
    auto drawOffset = [&](int ox, int oy, const QColor &col) {
        myPixels.setPixel(bx + ox * dir, by + oy, col);
    };

    const QColor body = b.bodyColor;
    const QColor wing = b.wingColor;
    const QColor belly = b.bellyColor;
    const QColor beak(243, 156, 18);
    const QColor eye(255, 255, 255);
    const QColor pupil(20, 20, 20);

    // Beak
    drawOffset(4, 0, beak);
    drawOffset(3, 0, beak);

    // Head & Eye
    drawOffset(2, 1, body);
    drawOffset(1, 1, eye);
    drawOffset(1, 1, pupil);
    drawOffset(2, 0, body);

    // Body
    for (int ox = -2; ox <= 1; ++ox)
    {
        drawOffset(ox, 0, body);
        drawOffset(ox, -1, belly);
    }
    drawOffset(-3, 0, body);   // Tail base
    drawOffset(-4, 1, wing);   // Tail feather

    // Flapping Wing
    if (b.wingFrame == 0)
    {
        // Wing Level / Up
        drawOffset(-1, 1, wing);
        drawOffset(-2, 1, wing);
        drawOffset(-1, 2, wing);
    }
    else
    {
        // Wing Down
        drawOffset(-1, -1, wing);
        drawOffset(-2, -1, wing);
        drawOffset(-1, -2, wing);
    }

    // Warning indicator if bird is about to lay an egg
    if (b.isLaying)
    {
        const QColor alertCol = (b.layingCountdown % 4 < 2) ? QColor(255, 193, 7) : QColor(231, 76, 60);
        myPixels.setPixel(bx, by - 2, alertCol);
        myPixels.setPixel(bx, by - 3, alertCol);
    }
}

void MainWindow::rasterizeEgg(const FallingEgg &egg)
{
    const int ex = static_cast<int>(std::round(egg.mathX));
    const int ey = static_cast<int>(std::round(egg.mathY));

    if (egg.type == EggType::REGULAR)
    {
        // 5x6 Regular White/Eggshell Egg
        const QColor outline(141, 110, 99);
        const QColor fill(255, 253, 240);
        const QColor shade(220, 214, 198);
        const QColor highlight(255, 255, 255);

        // Top row
        myPixels.setPixel(ex - 1, ey + 2, outline);
        myPixels.setPixel(ex, ey + 2, outline);
        myPixels.setPixel(ex + 1, ey + 2, outline);

        // Row 1
        myPixels.setPixel(ex - 2, ey + 1, outline);
        myPixels.setPixel(ex - 1, ey + 1, highlight);
        myPixels.setPixel(ex, ey + 1, fill);
        myPixels.setPixel(ex + 1, ey + 1, fill);
        myPixels.setPixel(ex + 2, ey + 1, outline);

        // Row 0
        myPixels.setPixel(ex - 2, ey, outline);
        myPixels.setPixel(ex - 1, ey, highlight);
        myPixels.setPixel(ex, ey, fill);
        myPixels.setPixel(ex + 1, ey, fill);
        myPixels.setPixel(ex + 2, ey, outline);

        // Row -1
        myPixels.setPixel(ex - 2, ey - 1, outline);
        myPixels.setPixel(ex - 1, ey - 1, fill);
        myPixels.setPixel(ex, ey - 1, fill);
        myPixels.setPixel(ex + 1, ey - 1, shade);
        myPixels.setPixel(ex + 2, ey - 1, outline);

        // Bottom row
        myPixels.setPixel(ex - 1, ey - 2, outline);
        myPixels.setPixel(ex, ey - 2, shade);
        myPixels.setPixel(ex + 1, ey - 2, outline);
    }
    else if (egg.type == EggType::GOLDEN)
    {
        // 6x7 Big Shiny Golden Egg
        const QColor border(160, 90, 0);
        const QColor goldBright(255, 235, 59);
        const QColor goldCore(255, 215, 0);
        const QColor goldDeep(255, 143, 0);
        const QColor sparkle = (egg.animTick % 6 < 3) ? QColor(255, 255, 255) : goldBright;

        // Top
        myPixels.setPixel(ex - 1, ey + 3, border);
        myPixels.setPixel(ex, ey + 3, border);

        for (int y = ey + 2; y >= ey - 2; --y)
        {
            myPixels.setPixel(ex - 2, y, border);
            myPixels.setPixel(ex + 2, y, border);
        }

        // Inner golden cells with animated sparkle
        myPixels.setPixel(ex - 1, ey + 2, sparkle);
        myPixels.setPixel(ex, ey + 2, goldBright);
        myPixels.setPixel(ex + 1, ey + 2, goldCore);

        myPixels.setPixel(ex - 1, ey + 1, goldBright);
        myPixels.setPixel(ex, ey + 1, goldCore);
        myPixels.setPixel(ex + 1, ey + 1, goldDeep);

        myPixels.setPixel(ex - 1, ey, goldCore);
        myPixels.setPixel(ex, ey, goldCore);
        myPixels.setPixel(ex + 1, ey, goldDeep);

        myPixels.setPixel(ex - 1, ey - 1, goldCore);
        myPixels.setPixel(ex, ey - 1, goldDeep);
        myPixels.setPixel(ex + 1, ey - 1, goldDeep);

        // Bottom
        myPixels.setPixel(ex - 1, ey - 2, border);
        myPixels.setPixel(ex, ey - 2, goldDeep);
        myPixels.setPixel(ex + 1, ey - 2, border);
    }
    else if (egg.type == EggType::BOMB)
    {
        // 6x7 False Egg (Bomb with flickering fuse)
        const QColor sparkColor = (egg.animTick % 4 < 2) ? QColor(255, 215, 0) : QColor(231, 76, 60);
        const QColor fuseColor(121, 85, 72);
        const QColor metalOutline(38, 50, 56);
        const QColor metalBody(55, 71, 79);
        const QColor highlight(120, 144, 156);
        const QColor dangerCross(229, 57, 53);

        // Flickering fuse on top
        myPixels.setPixel(ex, ey + 3, sparkColor);
        myPixels.setPixel(ex + 1, ey + 3, (egg.animTick % 3 == 0 ? sparkColor : fuseColor));
        myPixels.setPixel(ex, ey + 2, fuseColor);

        // Bomb body outline
        myPixels.setPixel(ex - 1, ey + 1, metalOutline);
        myPixels.setPixel(ex, ey + 1, metalOutline);
        myPixels.setPixel(ex + 1, ey + 1, metalOutline);

        for (int y = ey; y >= ey - 1; --y)
        {
            myPixels.setPixel(ex - 2, y, metalOutline);
            myPixels.setPixel(ex + 2, y, metalOutline);
        }

        // Bomb body interior with red danger cross
        myPixels.setPixel(ex - 1, ey, highlight);
        myPixels.setPixel(ex, ey, dangerCross);
        myPixels.setPixel(ex + 1, ey, metalBody);

        myPixels.setPixel(ex - 1, ey - 1, dangerCross);
        myPixels.setPixel(ex, ey - 1, dangerCross);
        myPixels.setPixel(ex + 1, ey - 1, dangerCross);

        // Bottom
        myPixels.setPixel(ex - 1, ey - 2, metalOutline);
        myPixels.setPixel(ex, ey - 2, metalOutline);
        myPixels.setPixel(ex + 1, ey - 2, metalOutline);
    }
}

void MainWindow::rasterizePixelHearts(int startX, int startY, int currentHearts, int maxHearts)
{
    // Draw 3 5x5 pixel hearts at upper left
    for (int h = 0; h < maxHearts; ++h)
    {
        const int hx = startX + h * 7;
        const int hy = startY;
        const bool active = (h < currentHearts);
        const QColor heartCol = active ? QColor(239, 71, 111) : QColor(50, 58, 76);

        // 5x5 Heart Pattern
        myPixels.setPixel(hx - 1, hy, heartCol);
        myPixels.setPixel(hx + 1, hy, heartCol);

        myPixels.setPixel(hx - 2, hy - 1, heartCol);
        myPixels.setPixel(hx - 1, hy - 1, heartCol);
        myPixels.setPixel(hx, hy - 1, heartCol);
        myPixels.setPixel(hx + 1, hy - 1, heartCol);
        myPixels.setPixel(hx + 2, hy - 1, heartCol);

        myPixels.setPixel(hx - 1, hy - 2, heartCol);
        myPixels.setPixel(hx, hy - 2, heartCol);
        myPixels.setPixel(hx + 1, hy - 2, heartCol);

        myPixels.setPixel(hx, hy - 3, heartCol);
    }
}

// Tiny 3x5 raster font used for HUD text drawn with pixels
int MainWindow::rasterizePixelText(int x, int topY, const QString &text, const QColor &color)
{
    static const std::unordered_map<char, std::array<const char *, 5>> glyphs = {
        {'0', {"###", "#.#", "#.#", "#.#", "###"}},
        {'1', {".#.", "##.", ".#.", ".#.", "###"}},
        {'2', {"###", "..#", "###", "#..", "###"}},
        {'3', {"###", "..#", "###", "..#", "###"}},
        {'4', {"#.#", "#.#", "###", "..#", "..#"}},
        {'5', {"###", "#..", "###", "..#", "###"}},
        {'6', {"###", "#..", "###", "#.#", "###"}},
        {'7', {"###", "..#", "..#", "..#", "..#"}},
        {'8', {"###", "#.#", "###", "#.#", "###"}},
        {'9', {"###", "#.#", "###", "..#", "###"}},
        {'A', {".#.", "#.#", "###", "#.#", "#.#"}},
        {'E', {"###", "#..", "##.", "#..", "###"}},
        {'L', {"#..", "#..", "#..", "#..", "###"}},
        {'M', {"#.#", "###", "###", "#.#", "#.#"}},
        {'O', {"###", "#.#", "#.#", "#.#", "###"}},
        {'P', {"###", "#.#", "###", "#..", "#.."}},
        {'S', {".##", "#..", ".#.", "..#", "##."}},
        {'T', {"###", ".#.", ".#.", ".#.", ".#."}},
        {'V', {"#.#", "#.#", "#.#", "#.#", ".#."}},
        {'X', {"#.#", "#.#", ".#.", "#.#", "#.#"}},
    };

    int cx = x;
    for (const QChar qc : text)
    {
        const char c = qc.toUpper().toLatin1();
        auto it = glyphs.find(c);
        if (it != glyphs.end())
        {
            for (int row = 0; row < 5; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    if (it->second[row][col] == '#')
                    {
                        myPixels.setPixel(cx + col, topY - row, color);
                    }
                }
            }
        }
        cx += 4; // 3px glyph + 1px spacing (unknown chars / spaces just advance)
    }
    return pixelTextWidth(text);
}

void MainWindow::rasterizeLevelBar(int minX, int maxX, int topY)
{
    // Layout (5 rows tall):  [========bar========]  40 PTS TO LV 3
    const bool maxed = level >= MAX_LEVEL;
    const int levelStart = (level - 1) * POINTS_PER_LEVEL;
    const int earned = maxed ? POINTS_PER_LEVEL : std::clamp(score - levelStart, 0, POINTS_PER_LEVEL);
    const int remaining = POINTS_PER_LEVEL - earned;

    // The bar occupies the space from minX to maxX exactly
    int barX0 = minX;
    int barX1 = maxX;
    if (barX1 - barX0 < 6)
    {
        return;
    }

    const QColor border(94, 72, 140);
    const QColor emptyCol(30, 34, 50);
    const QColor notchCol(22, 25, 38);
    const QColor fillA = maxed ? QColor(255, 143, 0) : QColor(123, 44, 191);
    const QColor fillB = maxed ? QColor(255, 215, 0) : QColor(199, 125, 255);

    // Border frame
    for (int x = barX0 + 1; x <= barX1 - 1; ++x)
    {
        myPixels.setPixel(x, topY, border);
        myPixels.setPixel(x, topY - 4, border);
    }
    for (int y = topY - 1; y >= topY - 3; --y)
    {
        myPixels.setPixel(barX0, y, border);
        myPixels.setPixel(barX1, y, border);
    }

    // Interior: filled portion (gradient + highlight + moving shimmer), empty portion with 10% notches
    const int innerX0 = barX0 + 1;
    const int innerX1 = barX1 - 1;
    const int innerW = innerX1 - innerX0 + 1;
    const int filledW = (innerW * earned) / POINTS_PER_LEVEL;
    const int shimmerX = innerX0 + (filledW > 0 ? (hudTick / 2) % (filledW + 8) : -100);

    for (int i = 0; i < innerW; ++i)
    {
        const int x = innerX0 + i;
        if (i < filledW)
        {
            const double t = innerW > 1 ? static_cast<double>(i) / (innerW - 1) : 1.0;
            QColor c(static_cast<int>(fillA.red() + (fillB.red() - fillA.red()) * t),
                     static_cast<int>(fillA.green() + (fillB.green() - fillA.green()) * t),
                     static_cast<int>(fillA.blue() + (fillB.blue() - fillA.blue()) * t));
            if (std::abs(x - shimmerX) <= 1)
            {
                c = c.lighter(140);
            }
            myPixels.setPixel(x, topY - 1, c.lighter(125)); // top highlight row
            myPixels.setPixel(x, topY - 2, c);
            myPixels.setPixel(x, topY - 3, c.darker(125));  // bottom shade row
        }
        else
        {
            const bool notch = ((i * 10) % innerW) < 10 && i > 0;
            const QColor c = notch ? notchCol : emptyCol;
            for (int y = topY - 1; y >= topY - 3; --y)
            {
                myPixels.setPixel(x, y, c);
            }
        }
    }


}

void MainWindow::spawnCatchParticles(double x, double y, const QColor &col, int count)
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

void MainWindow::spawnSplatParticles(double x, double y, EggType type)
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

void MainWindow::spawnExplosionParticles(double x, double y)
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

void MainWindow::addFloatingText(double x, double y, const QString &text, const QColor &col)
{
    FloatingText t;
    t.x = x;
    t.y = y;
    t.text = text;
    t.color = col;
    t.life = 28;
    floatingTexts.push_back(t);
}

void MainWindow::redrawPixels()
{
    const int width = ui->frame->width();
    const int height = ui->frame->height();
    if (width <= 10 || height <= 10)
    {
        return;
    }

    QPixmap pix(width, height);
    pix.fill(QColor(14, 17, 24)); // Dark arcade night canvas
    QPainter painter(&pix);

    myGrid.setDimensions(width, height);
    myGrid.setScale(scale);

    // 1. Clear raster pixel buffer and rasterize all game entities
    myPixels.clear();

    const int minMathX = myGrid.screenToMathX(0);
    const int maxMathX = myGrid.screenToMathX(width);
    const int minMathY = myGrid.screenToMathY(height);
    const int maxMathY = myGrid.screenToMathY(0);

    // Rasterize Ground
    rasterizeGround(minMathX, maxMathX, groundMathY, minMathY);

    // Rasterize Birds
    for (const auto &bird : birds)
    {
        rasterizeBird(bird);
    }

    // Rasterize Basket
    rasterizeBasket(basket);

    // Rasterize Falling Egg
    if (fallingEgg.active)
    {
        rasterizeEgg(fallingEgg);
    }

    // Rasterize Hearts in canvas top-left
    rasterizePixelHearts(minMathX + 3, maxMathY - 3, hearts, 3);

    // Rasterize Level Progress Bar in the top-right
    rasterizeLevelBar(maxMathX - 40, maxMathX - 3, maxMathY - 2);

    // 2. PASS 1: Pixel Glow Aura Pass (glowing aura radiating beyond cells without any grid lines)
    const int glowRadius = (scale <= 3 ? 1 : (scale <= 6 ? 2 : 3));
    const int glowAlpha = (scale <= 3 ? 32 : 45);

    // Special glowing aura for Golden Egg
    if (fallingEgg.active && fallingEgg.type == EggType::GOLDEN)
    {
        const int ex = myGrid.mathToScreenX(static_cast<int>(std::round(fallingEgg.mathX)));
        const int ey = myGrid.mathToScreenY(static_cast<int>(std::round(fallingEgg.mathY)));
        const int auraR = scale * 7;
        QRadialGradient goldGlow(ex + scale / 2.0, ey + scale / 2.0, auraR);
        goldGlow.setColorAt(0.0, QColor(255, 215, 0, 75));
        goldGlow.setColorAt(0.4, QColor(255, 190, 0, 28));
        goldGlow.setColorAt(1.0, QColor(255, 180, 0, 0));
        painter.fillRect(ex + scale / 2 - auraR, ey + scale / 2 - auraR, auraR * 2, auraR * 2, goldGlow);
    }
    else if (fallingEgg.active && fallingEgg.type == EggType::BOMB)
    {
        const int ex = myGrid.mathToScreenX(static_cast<int>(std::round(fallingEgg.mathX)));
        const int ey = myGrid.mathToScreenY(static_cast<int>(std::round(fallingEgg.mathY + 3.0)));
        const int sparkR = scale * 5;
        QRadialGradient sparkGlow(ex + scale / 2.0, ey + scale / 2.0, sparkR);
        sparkGlow.setColorAt(0.0, QColor(255, 120, 0, 85));
        sparkGlow.setColorAt(0.5, QColor(230, 60, 0, 30));
        sparkGlow.setColorAt(1.0, QColor(230, 50, 0, 0));
        painter.fillRect(ex + scale / 2 - sparkR, ey + scale / 2 - sparkR, sparkR * 2, sparkR * 2, sparkGlow);
    }

    // Glowing aura for all active raster pixels
    for (const auto &entry : myPixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        const int sx = myGrid.mathToScreenX(x);
        const int sy = myGrid.mathToScreenY(y);

        if (sx + scale + glowRadius < 0 || sx - glowRadius >= width ||
            sy + scale + glowRadius < 0 || sy - glowRadius >= height)
        {
            continue;
        }

        QColor glowColor = entry.second;
        glowColor.setAlpha(glowAlpha);
        painter.fillRect(sx - glowRadius, sy - glowRadius, scale + 2 * glowRadius, scale + 2 * glowRadius, glowColor);
    }

    // Glowing aura for particles
    for (const auto &p : particles)
    {
        const int sx = myGrid.mathToScreenX(static_cast<int>(std::round(p.x)));
        const int sy = myGrid.mathToScreenY(static_cast<int>(std::round(p.y)));
        const int pGlowR = glowRadius + 1;
        QColor pGlowColor = p.color;
        pGlowColor.setAlpha(65);
        painter.fillRect(sx - pGlowR, sy - pGlowR, scale + 2 * pGlowR, scale + 2 * pGlowR, pGlowColor);
    }

    // 3. PASS 2: Pixel Core Pass (draws crisp solid cells with inner phosphor brightness, no grid lines)
    for (const auto &entry : myPixels.getPixelMap())
    {
        const int x = static_cast<int>(entry.first >> 32);
        const int y = static_cast<int>(entry.first & 0xFFFFFFFFLL);
        paintCell(painter, x, y, entry.second);
    }

    // Dynamic particles core pass
    for (const auto &p : particles)
    {
        paintCell(painter, static_cast<int>(std::round(p.x)), static_cast<int>(std::round(p.y)), p.color);
    }

    // 5. Paint Floating Texts
    QFont font("Segoe UI", 12, QFont::Bold);
    painter.setFont(font);
    for (const auto &t : floatingTexts)
    {
        const int sx = myGrid.mathToScreenX(static_cast<int>(std::round(t.x)));
        const int sy = myGrid.mathToScreenY(static_cast<int>(std::round(t.y)));

        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(sx - 39, sy + 1, t.text);
        painter.setPen(t.color);
        painter.drawText(sx - 40, sy, t.text);
    }

    // Draw Progress Bar Text below the bar
    if (gameState == GameState::PLAYING) {
        const bool maxed = level >= MAX_LEVEL;
        const int levelStart = (level - 1) * POINTS_PER_LEVEL;
        const int earned = maxed ? POINTS_PER_LEVEL : std::clamp(score - levelStart, 0, POINTS_PER_LEVEL);
        const int remaining = POINTS_PER_LEVEL - earned;
        const QString progressLabel = maxed ? QString("MAX LEVEL")
                                            : QString("%1 PTS TO NEXT LEVEL").arg(remaining);

        QFont progressFont("Segoe UI", 9, QFont::Bold);
        painter.setFont(progressFont);
        
        const int sx = myGrid.mathToScreenX(maxMathX - 40);
        const int sy = myGrid.mathToScreenY(maxMathY - 8);
        const int barW = myGrid.mathToScreenX(maxMathX - 3) - sx;
        
        painter.setPen(QColor(0, 0, 0, 180));
        painter.drawText(QRect(sx + 1, sy + 1, barW, 20), Qt::AlignCenter, progressLabel);
        
        painter.setPen(maxed ? QColor(255, 215, 0) : QColor(224, 230, 237));
        painter.drawText(QRect(sx, sy, barW, 20), Qt::AlignCenter, progressLabel);
    }

    // 6. Level-up banner (fades out)
    if (levelBannerTicks > 0 && gameState == GameState::PLAYING)
    {
        const int alpha = std::min(255, levelBannerTicks * 8);
        painter.setPen(QColor(0, 0, 0, alpha * 2 / 3));
        QFont bannerFont("Segoe UI", 34, QFont::Black);
        painter.setFont(bannerFont);
        const QRect bannerRect(0, height / 3 - 40, width, 70);
        painter.drawText(bannerRect.translated(3, 3), Qt::AlignCenter, QString("LEVEL %1").arg(level));
        painter.setPen(QColor(199, 125, 255, alpha));
        painter.drawText(bannerRect, Qt::AlignCenter, QString("LEVEL %1").arg(level));

        QFont subFont("Segoe UI", 13, QFont::DemiBold);
        painter.setFont(subFont);
        painter.setPen(QColor(224, 230, 237, alpha));
        painter.drawText(QRect(0, height / 3 + 25, width, 30), Qt::AlignCenter,
                         "Faster eggs • More bombs • Smaller basket");
    }

    // 7. Overlays for Game Over and Paused
    if (gameState == GameState::GAME_OVER)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 210));

        painter.setPen(QColor(239, 71, 111));
        QFont titleFont("Segoe UI", 32, QFont::Bold);
        painter.setFont(titleFont);
        painter.drawText(QRect(0, height / 2 - 110, width, 60), Qt::AlignCenter, "GAME OVER");

        painter.setPen(QColor(255, 209, 102));
        QFont subFont("Segoe UI", 16, QFont::DemiBold);
        painter.setFont(subFont);
        painter.drawText(QRect(0, height / 2 - 40, width, 40), Qt::AlignCenter, gameOverReason);

        painter.setPen(QColor(6, 214, 160));
        QFont scoreFont("Segoe UI", 15, QFont::Normal);
        painter.setFont(scoreFont);
        painter.drawText(QRect(0, height / 2 + 10, width, 30), Qt::AlignCenter,
                         QString("Final Score: %1    |    Level Reached: %2    |    High Score: %3")
                             .arg(score).arg(level).arg(highScore));

        painter.setPen(QColor(168, 199, 250));
        QFont promptFont("Segoe UI", 13, QFont::Normal);
        painter.setFont(promptFont);
        painter.drawText(QRect(0, height / 2 + 60, width, 30), Qt::AlignCenter,
                         "Click, press [SPACE] or [R], or click 'Restart' to play again!");
    }
    else if (gameState == GameState::PAUSED)
    {
        painter.fillRect(0, 0, width, height, QColor(10, 12, 18, 175));

        painter.setPen(QColor(255, 209, 102));
        QFont pauseFont("Segoe UI", 28, QFont::Bold);
        painter.setFont(pauseFont);
        painter.drawText(QRect(0, height / 2 - 50, width, 50), Qt::AlignCenter, "PAUSED");

        painter.setPen(QColor(224, 230, 237));
        QFont promptFont("Segoe UI", 13, QFont::Normal);
        painter.setFont(promptFont);
        painter.drawText(QRect(0, height / 2 + 15, width, 30), Qt::AlignCenter,
                         "Click, press [SPACE], or click 'Resume' to continue");
    }

    painter.end();
    ui->frame->setPixmap(pix);
}

void MainWindow::updateHUD()
{
    ui->lblScore->setText(QString("SCORE: %1").arg(score));
    ui->lblHighScore->setText(QString("BEST: %1").arg(highScore));

    QString heartsText;
    for (int i = 0; i < 3; ++i)
    {
        heartsText += (i < hearts) ? "❤" : "♡";
    }
    ui->lblHearts->setText(heartsText);

    if (lblLevel)
    {
        lblLevel->setText(level >= MAX_LEVEL ? QString("LEVEL %1 (MAX)").arg(level)
                                             : QString("LEVEL %1").arg(level));
    }
}

void MainWindow::on_btnRestart_clicked()
{
    resetGame();
}

void MainWindow::on_btnPause_clicked()
{
    togglePause();
}

void MainWindow::on_spinScale_valueChanged(int val)
{
    scale = val;
    myGrid.setScale(scale);
    redrawPixels();
}

void MainWindow::onGridPanned(int dx, int dy)
{
    myGrid.pan(dx, dy);
    redrawPixels();
}

// -------------------------------------------------------------
// Mouse Controls
// -------------------------------------------------------------
void MainWindow::onMouseMoved(QPoint &pos)
{
    // Basket follows the cursor horizontally. Tiny jitter is ignored so a resting
    // mouse never steals control back from the keyboard.
    const int dx = std::abs(pos.x() - lastMouseX);
    lastMouseX = pos.x();
    if (mouseControl || dx >= 3)
    {
        mouseScreenX = pos.x();
        mouseControl = true;
    }
}

void MainWindow::onMouseLeftClicked()
{
    ui->frame->setFocus(); // keep keyboard working after clicking the canvas
    mouseScreenX = ui->frame->lastClickPosition().x();
    mouseControl = true;

    // Left-click resumes a paused game or restarts after game over
    if (gameState == GameState::PAUSED || gameState == GameState::GAME_OVER)
    {
        togglePause();
    }
}

void MainWindow::onMouseRightClicked()
{
    togglePause();
}
