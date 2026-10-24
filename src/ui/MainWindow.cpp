#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "CanvasLabel.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QCursor>
#include <algorithm>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Eggscellent Catch");

    scale = 4;
    myGrid.setScale(scale);

    this->installEventFilter(this);

    connect(ui->frame, &CanvasLabel::panDelta, this, &MainWindow::onGridPanned);
    connect(ui->frame, &CanvasLabel::sendMousePosition, this, &MainWindow::onMouseMoved);
    connect(ui->frame, &CanvasLabel::Mouse_Pos, this, &MainWindow::onMouseLeftClicked);
    connect(ui->frame, &CanvasLabel::rightClicked, this, &MainWindow::onMouseRightClicked);

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoopTick);
    gameTimer->start(30);

    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    engine.init(myGrid);
    engine.setState(GameState::MENU);

    // Initialize Audio Manager & Connect Engine Sound Callback
    audioManager.init();
    engine.setSoundCallback([this](SoundId sfx) {
        if (sfx == SoundId::GAME_OVER) {
            audioManager.stopBGM();
            unlockCursor();
        }
        audioManager.playSound(sfx);
    });

    updateHUD();
}

MainWindow::~MainWindow()
{
    unlockCursor();
    if (gameTimer)
    {
        gameTimer->stop();
    }
    audioManager.stopBGM();
    delete ui;
}

void MainWindow::lockCursorToFrame()
{
#ifdef Q_OS_WIN
    if (ui && ui->frame && isVisible())
    {
        HWND hwnd = reinterpret_cast<HWND>(ui->frame->winId());
        RECT clientRect;
        if (hwnd && GetClientRect(hwnd, &clientRect))
        {
            POINT ptTopLeft = {clientRect.left, clientRect.top};
            ClientToScreen(hwnd, &ptTopLeft);

            POINT ptBottomRight = {clientRect.right, clientRect.bottom};
            ClientToScreen(hwnd, &ptBottomRight);

            RECT physicalRect;
            physicalRect.left = ptTopLeft.x;
            physicalRect.top = ptTopLeft.y;
            physicalRect.right = ptBottomRight.x;
            physicalRect.bottom = ptBottomRight.y;

            ClipCursor(&physicalRect);
        }
        else
        {
            const double dpr = devicePixelRatio();
            const QPoint tl = ui->frame->mapToGlobal(QPoint(0, 0));
            const QPoint br = ui->frame->mapToGlobal(QPoint(ui->frame->width(), ui->frame->height()));
            RECT rect;
            rect.left = static_cast<LONG>(std::round(tl.x() * dpr));
            rect.top = static_cast<LONG>(std::round(tl.y() * dpr));
            rect.right = static_cast<LONG>(std::round(br.x() * dpr));
            rect.bottom = static_cast<LONG>(std::round(br.y() * dpr));
            ClipCursor(&rect);
        }
    }
#endif
    if (ui && ui->frame)
    {
        ui->frame->setCursor(Qt::BlankCursor);
    }
    this->setCursor(Qt::BlankCursor);
}

void MainWindow::unlockCursor()
{
#ifdef Q_OS_WIN
    ClipCursor(NULL);
#endif
    if (ui && ui->frame)
    {
        ui->frame->setCursor(Qt::ArrowCursor);
    }
    this->setCursor(Qt::ArrowCursor);
}

void MainWindow::nextMenuSlide()
{
    menuSlideIndex = (menuSlideIndex + 1) % 3;
    engine.setMenuSlideIndex(menuSlideIndex);
    audioManager.playSound(SoundId::UI_CLICK);
    redrawPixels();
}

void MainWindow::startGame()
{
    engine.setState(GameState::PLAYING);
    audioManager.playSound(SoundId::UI_CLICK);
    audioManager.playBGM();

    lockCursorToFrame();

    updateHUD();
    redrawPixels();
}

void MainWindow::resetGame()
{
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    engine.reset(myGrid);
    audioManager.playSound(SoundId::UI_CLICK);
    audioManager.stopBGM();
    audioManager.playBGM();

    lockCursorToFrame();

    updateHUD();
    redrawPixels();
}

void MainWindow::togglePause()
{
    if (engine.getState() == GameState::MENU)
    {
        return;
    }
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    engine.togglePause();
    audioManager.playSound(SoundId::UI_CLICK);
    if (engine.getState() == GameState::PAUSED)
    {
        audioManager.pauseBGM();
        unlockCursor();
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        audioManager.resumeBGM();
        lockCursorToFrame();
    }
    updateHUD();
    redrawPixels();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    if (engine.getState() == GameState::PLAYING)
    {
        lockCursorToFrame();
    }
    redrawPixels();
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    QMainWindow::moveEvent(event);
    if (engine.getState() == GameState::PLAYING)
    {
        lockCursorToFrame();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange)
    {
        if (!isActiveWindow())
        {
            unlockCursor();
            if (engine.getState() == GameState::PLAYING)
            {
                togglePause();
            }
        }
        else if (engine.getState() == GameState::PLAYING)
        {
            lockCursorToFrame();
        }
    }
    QMainWindow::changeEvent(event);
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

    if (engine.getState() == GameState::MENU)
    {
        if (key == Qt::Key_1)
        {
            engine.setMode(GameMode::EASY, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (key == Qt::Key_2)
        {
            engine.setMode(GameMode::MEDIUM, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (key == Qt::Key_3)
        {
            engine.setMode(GameMode::HARD, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (key == Qt::Key_Tab)
        {
            engine.cycleMode(myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (key == Qt::Key_M || key == Qt::Key_S)
        {
            audioManager.toggleMute();
            audioManager.playSound(SoundId::UI_CLICK);
            engine.setStatusMessage(audioManager.isMuted() ? "🔇 Audio Muted. Press [M] to unmute." : "🔊 Audio Unmuted! Enjoy the chill cheerful tunes!");
            redrawPixels();
            return;
        }
        else if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter)
        {
            startGame();
            return;
        }
        else if (key == Qt::Key_Right)
        {
            nextMenuSlide();
            return;
        }
    }
    else if (engine.getState() == GameState::GAME_OVER)
    {
        if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_R)
        {
            resetGame();
            return;
        }
    }

    // Toggle Pause with ESC key (or Space while paused)
    if (key == Qt::Key_Escape || (key == Qt::Key_Space && engine.getState() == GameState::PAUSED))
    {
        togglePause();
        return;
    }

    if (key == Qt::Key_H)
    {
        engine.useGrowthEgg();
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_C)
    {
        engine.summonCompanionBasket();
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_1)
    {
        engine.setMode(GameMode::EASY, myGrid);
        audioManager.playSound(SoundId::UI_CLICK);
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_2)
    {
        engine.setMode(GameMode::MEDIUM, myGrid);
        audioManager.playSound(SoundId::UI_CLICK);
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_3)
    {
        engine.setMode(GameMode::HARD, myGrid);
        audioManager.playSound(SoundId::UI_CLICK);
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_Tab)
    {
        engine.cycleMode(myGrid);
        audioManager.playSound(SoundId::UI_CLICK);
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_M || key == Qt::Key_S)
    {
        audioManager.toggleMute();
        audioManager.playSound(SoundId::UI_CLICK);
        engine.setStatusMessage(audioManager.isMuted() ? "🔇 Audio Muted. Press [M] to unmute." : "🔊 Audio Unmuted! Enjoy the chill cheerful tunes!");
        redrawPixels();
        return;
    }
    else if (key == Qt::Key_Left || key == Qt::Key_A)
    {
        engine.setKeyLeft(true);
    }
    else if (key == Qt::Key_Right || key == Qt::Key_D)
    {
        engine.setKeyRight(true);
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
        engine.setKeyLeft(false);
    }
    else if (key == Qt::Key_Right || key == Qt::Key_D)
    {
        engine.setKeyRight(false);
    }
    else
    {
        QMainWindow::keyReleaseEvent(event);
    }
}

void MainWindow::gameLoopTick()
{
    hudTick++;
    const GameState prevState = engine.getState();
    engine.tick(myGrid);

    if (engine.getState() == GameState::GAME_OVER)
    {
        if (prevState == GameState::PLAYING)
        {
            unlockCursor();
        }
    }
    else if (engine.getState() == GameState::PAUSED || engine.getState() == GameState::MENU)
    {
        unlockCursor();
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        // Keep cursor hidden while playing
        if (ui && ui->frame && ui->frame->cursor().shape() != Qt::BlankCursor)
        {
            ui->frame->setCursor(Qt::BlankCursor);
        }

        if (engine.isMouseControl() && ui && ui->frame)
        {
            const QPoint localPos = ui->frame->mapFromGlobal(QCursor::pos());
            const int clampedX = std::clamp(localPos.x(), 0, std::max(1, ui->frame->width() - 1));
            engine.handleMouseMove(clampedX);
        }
    }

    updateHUD();
    redrawPixels();
}

void MainWindow::updateHUD()
{
    // HUD is rendered via GameRenderer
}

void MainWindow::redrawPixels()
{
    const int width = ui->frame->width();
    const int height = ui->frame->height();
    if (width <= 10 || height <= 10)
    {
        return;
    }

    QPixmap pix = renderer.renderFrame(engine, myGrid, myPixels, width, height, scale, hudTick);
    ui->frame->setPixmap(pix);
}

void MainWindow::onGridPanned(int dx, int dy)
{
    myGrid.pan(dx, dy);
    redrawPixels();
}

void MainWindow::onMouseMoved(QPoint &pos)
{
    if (engine.getState() == GameState::PLAYING)
    {
        engine.handleMouseMove(pos.x());
    }
}

void MainWindow::onMouseLeftClicked()
{
    if (engine.getState() == GameState::MENU)
    {
        const int width = ui->frame->width();
        const int height = ui->frame->height();
        const int cardW = std::min(width - 40, 520);
        const int cardH = std::min(height - 40, 390);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;
        const int modeBtnW = 100;
        const int modeBtnH = 26;
        const int modeBarY = cardY + 75;
        const int startModeX = cardX + (cardW - (modeBtnW * 3 + 20)) / 2;

        const QPoint clickPos = ui->frame->lastClickPosition();
        if (clickPos.y() >= modeBarY && clickPos.y() <= modeBarY + modeBtnH)
        {
            if (clickPos.x() >= startModeX && clickPos.x() <= startModeX + modeBtnW)
            {
                engine.setMode(GameMode::EASY, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            else if (clickPos.x() >= startModeX + modeBtnW + 10 && clickPos.x() <= startModeX + modeBtnW * 2 + 10)
            {
                engine.setMode(GameMode::MEDIUM, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            else if (clickPos.x() >= startModeX + (modeBtnW + 10) * 2 && clickPos.x() <= startModeX + (modeBtnW + 10) * 2 + modeBtnW)
            {
                engine.setMode(GameMode::HARD, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
        }

        startGame();
        return;
    }
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    if (engine.getState() == GameState::PAUSED)
    {
        togglePause();
        return;
    }

    engine.handleMouseLeftClick(ui->frame->lastClickPosition().x());
    updateHUD();
}

void MainWindow::onMouseRightClicked()
{
    togglePause();
}
