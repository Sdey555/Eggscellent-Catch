#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QColor>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <vector>
#include <unordered_map>
#include <random>
#include <cmath>
#include <algorithm>

namespace Ui {
class MainWindow;
}

// -------------------------------------------------------------
// Core Raster Pixel Logic
// -------------------------------------------------------------
class MyPix
{
public:
    explicit MyPix(int x = 0, int y = 0, const QColor &color = QColor(255, 255, 255))
        : x(x), y(y), color(color) {}

    int getX() const { return x; }
    int getY() const { return y; }
    QColor getColor() const { return color; }
    void setColor(const QColor &c) { color = c; }

private:
    int x;
    int y;
    QColor color;
};

class MyPixels
{
public:
    static inline long long makeKey(int x, int y)
    {
        return (static_cast<long long>(x) << 32) | static_cast<unsigned int>(y);
    }

    void setPixel(int x, int y, const QColor &color)
    {
        pixelMap[makeKey(x, y)] = color;
    }

    void clear()
    {
        pixelMap.clear();
    }

    bool hasPixel(int x, int y) const
    {
        return pixelMap.find(makeKey(x, y)) != pixelMap.end();
    }

    bool getPixelColor(int x, int y, QColor &outColor) const
    {
        auto it = pixelMap.find(makeKey(x, y));
        if (it != pixelMap.end()) {
            outColor = it->second;
            return true;
        }
        return false;
    }

    const std::unordered_map<long long, QColor>& getPixelMap() const
    {
        return pixelMap;
    }

private:
    std::unordered_map<long long, QColor> pixelMap;
};

// -------------------------------------------------------------
// Core Raster Grid Logic (Axes rendering removed, coordinate logic kept)
// -------------------------------------------------------------
class MyGrid
{
public:
    MyGrid()
        : width(0),
          height(0),
          scale(4),
          originX(0),
          originY(0),
          panOffsetX(0),
          panOffsetY(0)
    {}

    void setDimensions(int w, int h)
    {
        width = w;
        height = h;
        calculateOrigin();
    }

    void setScale(int newScale)
    {
        scale = newScale;
        calculateOrigin();
    }

    int getScale() const { return scale; }
    int getOriginX() const { return originX; }
    int getOriginY() const { return originY; }
    int getPanOffsetX() const { return panOffsetX; }
    int getPanOffsetY() const { return panOffsetY; }

    void pan(int dx, int dy)
    {
        panOffsetX += dx;
        panOffsetY += dy;
        calculateOrigin();
    }

    void resetPan()
    {
        panOffsetX = 0;
        panOffsetY = 0;
        calculateOrigin();
    }

    int screenToMathX(int screenX) const
    {
        return static_cast<int>(
            std::floor(static_cast<double>(screenX - originX) / scale));
    }

    int screenToMathY(int screenY) const
    {
        return static_cast<int>(
            std::floor(static_cast<double>(originY - screenY - 1.0) / scale));
    }

    int mathToScreenX(int mathX) const
    {
        return originX + mathX * scale;
    }

    int mathToScreenY(int mathY) const
    {
        return originY - (mathY + 1) * scale;
    }

private:
    int width;
    int height;
    int scale;
    int originX;
    int originY;
    int panOffsetX{0};
    int panOffsetY{0};

    void calculateOrigin()
    {
        if (scale <= 0)
            return;
        int baseOriginX = static_cast<int>(std::round((width / 2.0) / scale) * scale);
        int baseOriginY = static_cast<int>(std::round((height / 2.0) / scale) * scale);
        originX = baseOriginX + panOffsetX;
        originY = baseOriginY + panOffsetY;
    }
};

// -------------------------------------------------------------
// Egg Catcher Game Structs & Types
// -------------------------------------------------------------
enum class EggType {
    REGULAR,    // Standard egg (+10 pts, miss = -1 heart)
    GOLDEN,     // Special bonus egg (+50 pts, miss = -1 heart)
    BOMB        // False egg (bomb: catch = instant game over, miss = safe!)
};

enum class GameState {
    PLAYING,
    PAUSED,
    GAME_OVER
};

struct FallingEgg {
    double mathX = 0.0;
    double mathY = 0.0;
    double vy = 0.08;          // current downward velocity
    double gravity = 0.015;    // velocity increment for gravity effect
    double maxSpeed = 1.6;     // terminal velocity
    EggType type = EggType::REGULAR;
    bool active = false;
    int animTick = 0;
};

struct Bird {
    double mathX = 0.0;
    double mathY = 0.0;
    double speed = 0.28;
    int direction = 1; // +1 right, -1 left
    int wingFrame = 0;
    int wingTick = 0;
    QColor bodyColor;
    QColor wingColor;
    QColor bellyColor;
    bool isLaying = false;
    int layingCountdown = 0;
    EggType pendingEggType = EggType::REGULAR;
};

struct Basket {
    double mathX = 0.0;
    int mathY = 0;
    int halfWidth = 5;
    int height = 4;
    double speed = 1.15;
};

struct GameParticle {
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    QColor color;
    int life = 0;
    int maxLife = 0;
};

struct FloatingText {
    double x = 0.0;
    double y = 0.0;
    QString text;
    QColor color;
    int life = 0;
};

// -------------------------------------------------------------
// MainWindow Class
// -------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void gameLoopTick();
    void on_btnRestart_clicked();
    void on_btnPause_clicked();
    void on_spinScale_valueChanged(int val);
    void onGridPanned(int dx, int dy);

private:
    Ui::MainWindow *ui;

    // Raster Grid & Raster Pixels
    int scale = 4;
    MyGrid myGrid;
    MyPixels myPixels;

    // Game Core
    GameState gameState = GameState::PLAYING;
    int score = 0;
    int highScore = 0;
    int hearts = 3;
    QString gameOverReason;

    Basket basket;
    std::vector<Bird> birds;
    FallingEgg fallingEgg;
    std::vector<GameParticle> particles;
    std::vector<FloatingText> floatingTexts;

    int eggSpawnCooldown = 100;
    int groundMathY = -24;

    // Input States
    bool keyLeftPressed = false;
    bool keyRightPressed = false;

    // Game Timer
    QTimer *gameTimer = nullptr;
    std::mt19937 rng;

    // Game Logic Methods
    void initGame();
    void resetGame();
    void updatePhysics();
    void updateBirds();
    void updateEggSpawn();
    void updateFallingEgg();
    void updateParticles();
    void redrawPixels();

    // Raster Painting Methods
    void paintCell(QPainter &painter, int mathX, int mathY, const QColor &color);
    void rasterizeGround(int minX, int maxX, int gY, int minY);
    void rasterizeBasket(const Basket &b);
    void rasterizeBird(const Bird &b);
    void rasterizeEgg(const FallingEgg &egg);
    void rasterizePixelHearts(int startX, int startY, int currentHearts, int maxHearts);

    // Particle / Effect Generators
    void spawnCatchParticles(double x, double y, const QColor &col, int count = 14);
    void spawnSplatParticles(double x, double y, EggType type);
    void spawnExplosionParticles(double x, double y);
    void addFloatingText(double x, double y, const QString &text, const QColor &col);

    // HUD Update
    void updateHUD();
};

#endif // MAINWINDOW_H
