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
    connect(ui->frame, &CanvasLabel::leftReleased, this, &MainWindow::onMouseLeftReleased);
    connect(ui->frame, &CanvasLabel::rightClicked, this, &MainWindow::onMouseRightClicked);
    connect(ui->frame, &CanvasLabel::dragScrolled, this, &MainWindow::onDragScrolled);
    connect(ui->frame, &CanvasLabel::wheelScrolled, this, &MainWindow::onWheelScrolled);

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
#ifndef Q_OS_ANDROID
    if (engine.getControlMode() != ControlMode::MOUSE_KEYBOARD)
    {
        return;
    }
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
#endif
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

void MainWindow::prevMenuSlide()
{
    menuSlideIndex = (menuSlideIndex + 2) % 3;
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

void MainWindow::returnToMainMenu()
{
    menuSlideIndex = 0;
    engine.setMenuSlideIndex(0);
    engine.setState(GameState::MENU);
    engine.setMenuScrollY(0);
    engine.reset(myGrid);
    unlockCursor();
    audioManager.playSound(SoundId::UI_CLICK);
    audioManager.stopBGM();
    audioManager.playBGM();
    updateHUD();
    redrawPixels();
}

void MainWindow::openSettings()
{
    previousStateBeforeSettings = engine.getState();
    engine.setState(GameState::SETTINGS);
    engine.setSettingsScrollY(0);
    unlockCursor();
    audioManager.playSound(SoundId::UI_CLICK);
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
    if (engine.getState() == GameState::SETTINGS)
    {
        engine.setState(previousStateBeforeSettings);
        if (previousStateBeforeSettings == GameState::PLAYING)
        {
            audioManager.resumeBGM();
            lockCursorToFrame();
        }
        audioManager.playSound(SoundId::UI_CLICK);
        updateHUD();
        redrawPixels();
        return;
    }
    engine.togglePause();
    audioManager.playSound(SoundId::UI_CLICK);
    if (engine.getState() == GameState::PAUSED)
    {
        engine.setPauseScrollY(0);
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
        else if (key == Qt::Key_Left)
        {
            prevMenuSlide();
            return;
        }
        else if (key == Qt::Key_Right)
        {
            nextMenuSlide();
            return;
        }
        else if (key == Qt::Key_Escape || key == Qt::Key_Back || key == Qt::Key_Q)
        {
            QApplication::quit();
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
        else if (key == Qt::Key_Escape || key == Qt::Key_Back || key == Qt::Key_Q)
        {
            QApplication::quit();
            return;
        }
    }
    else if (engine.getState() == GameState::SETTINGS)
    {
        if (key == Qt::Key_Escape || key == Qt::Key_Backspace || key == Qt::Key_Back)
        {
            togglePause();
            return;
        }
    }
    else if (engine.getState() == GameState::PAUSED)
    {
        if (key == Qt::Key_Escape || key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter)
        {
            togglePause();
            return;
        }
        else if (key == Qt::Key_S)
        {
            openSettings();
            return;
        }
        else if (key == Qt::Key_R)
        {
            resetGame();
            return;
        }
        else if (key == Qt::Key_M)
        {
            returnToMainMenu();
            return;
        }
        else if (key == Qt::Key_Q || key == Qt::Key_X)
        {
            QApplication::quit();
            return;
        }
    }

    // Toggle Pause with ESC key
    if (key == Qt::Key_Escape)
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
    else if (engine.getState() == GameState::PAUSED || engine.getState() == GameState::MENU || engine.getState() == GameState::SETTINGS)
    {
        unlockCursor();
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        if (engine.getControlMode() == ControlMode::MOUSE_KEYBOARD)
        {
            // Keep cursor hidden while playing in Mouse mode
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
        else
        {
            // For Touch, Virtual Buttons, or Gyro, keep cursor normal/visible
            if (ui && ui->frame && ui->frame->cursor().shape() == Qt::BlankCursor)
            {
                ui->frame->setCursor(Qt::ArrowCursor);
            }
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

    QPixmap pix = renderer.renderFrame(engine, myGrid, myPixels, width, height, scale, hudTick, &audioManager);
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
        if (engine.getControlMode() == ControlMode::VIRTUAL_BUTTONS)
        {
            if (ui && ui->frame && ui->frame->isLeftMouseDown())
            {
                const int width = ui->frame->width();
                const int height = ui->frame->height();
                const QRect leftRect = GameRenderer::getVirtualLeftButtonRect(width, height);
                const QRect rightRect = GameRenderer::getVirtualRightButtonRect(width, height);
                engine.setKeyLeft(leftRect.contains(pos));
                engine.setKeyRight(rightRect.contains(pos));
            }
            return;
        }
        else if (engine.getControlMode() == ControlMode::GYROSCOPE)
        {
            return;
        }

        engine.handleMouseMove(pos.x());
    }
}

void MainWindow::onDragScrolled(int dy)
{
    const int height = ui->frame->height();
    if (engine.getState() == GameState::SETTINGS)
    {
        const int cardH = std::min(height - 40, 350);
        const int viewportH = cardH - 60;
        const int totalContentH = 325;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        engine.scrollSettings(-dy, maxScroll);
        redrawPixels();
    }
    else if (engine.getState() == GameState::PAUSED)
    {
        const int cardH = std::min(height - 40, 340);
        const int viewportH = cardH - 64;
        const int totalContentH = 250;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        engine.scrollPause(-dy, maxScroll);
        redrawPixels();
    }
    else if (engine.getState() == GameState::MENU)
    {
        const int cardH = std::min(height - 40, 360);
        const int viewportH = cardH - 62;
        const int totalContentH = 370;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        engine.scrollMenu(-dy, maxScroll);
        redrawPixels();
    }
}

void MainWindow::onWheelScrolled(int delta)
{
    const int step = (delta > 0) ? -32 : 32;
    onDragScrolled(step);
}

void MainWindow::onMouseLeftClicked()
{
    const int width = ui->frame->width();
    const int height = ui->frame->height();
    const QPoint clickPos = ui->frame->lastClickPosition();

    // 1. MENU STATE
    if (engine.getState() == GameState::MENU)
    {
        const int cardW = std::min(width - 40, 520);
        const int cardH = std::min(height - 40, 360);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        // Check Settings Button on Menu Card Header
        const QRect menuSettingsBtn(cardX + cardW - 95, cardY + 12, 80, 26);
        if (menuSettingsBtn.contains(clickPos))
        {
            openSettings();
            return;
        }

        // Check Exit Button on Menu Card Header
        const QRect menuExitBtn(cardX + 15, cardY + 12, 65, 26);
        if (menuExitBtn.contains(clickPos))
        {
            audioManager.playSound(SoundId::UI_CLICK);
            QApplication::quit();
            return;
        }

        const int contentTop = cardY + 54;
        const int viewportH = cardH - 62;
        const int totalContentH = 370;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        const int scrollY = std::clamp(engine.getMenuScrollY(), 0, maxScroll);

        // Click on Scrollbar Track
        if (maxScroll > 0 && clickPos.x() >= cardX + cardW - 16 && clickPos.x() <= cardX + cardW - 2
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const float ratio = static_cast<float>(clickPos.y() - contentTop) / viewportH;
            engine.setMenuScrollY(static_cast<int>(ratio * maxScroll));
            redrawPixels();
            return;
        }

        // Viewport click
        if (clickPos.x() >= cardX + 4 && clickPos.x() <= cardX + cardW - 16
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const QPoint logicalPos(clickPos.x(), clickPos.y() + scrollY);

            // Mode pills
            const int modeBtnW = 100;
            const int modeBtnH = 26;
            const int modeBarY = contentTop + 8;
            const int startModeX = cardX + (cardW - (modeBtnW * 3 + 20)) / 2;

            if (logicalPos.y() >= modeBarY && logicalPos.y() <= modeBarY + modeBtnH)
            {
                if (logicalPos.x() >= startModeX && logicalPos.x() <= startModeX + modeBtnW)
                {
                    engine.setMode(GameMode::EASY, myGrid);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
                else if (logicalPos.x() >= startModeX + modeBtnW + 10 && logicalPos.x() <= startModeX + modeBtnW * 2 + 10)
                {
                    engine.setMode(GameMode::MEDIUM, myGrid);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
                else if (logicalPos.x() >= startModeX + (modeBtnW + 10) * 2 && logicalPos.x() <= startModeX + (modeBtnW + 10) * 2 + modeBtnW)
                {
                    engine.setMode(GameMode::HARD, myGrid);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
            }

            // Dedicated Start Game Button
            const int startBtnW = std::min(cardW - 70, 320);
            const int startBtnH = 34;
            const int startBtnX = cardX + (cardW - startBtnW) / 2;
            const int startBtnY = modeBarY + 54;
            const QRect startBtnRect(startBtnX, startBtnY, startBtnW, startBtnH);
            if (startBtnRect.contains(logicalPos))
            {
                startGame();
                return;
            }

            // Tap anywhere on slide guide / pagination to advance slide
            const int slideGuideY = startBtnY + 44;
            if (logicalPos.y() >= slideGuideY)
            {
                if (logicalPos.x() < cardX + cardW / 2)
                {
                    prevMenuSlide();
                }
                else
                {
                    nextMenuSlide();
                }
                return;
            }
        }

        // Tapping inside the card frame outside buttons does nothing (doesn't accidentally start)
        const QRect cardRect(cardX, cardY, cardW, cardH);
        if (cardRect.contains(clickPos))
        {
            return;
        }

        // Tapping outside starts game
        startGame();
        return;
    }

    // 2. GAME OVER STATE
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }

    // 3. PAUSED STATE
    if (engine.getState() == GameState::PAUSED)
    {
        const int cardW = std::min(width - 40, 420);
        const int cardH = std::min(height - 40, 340);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        const int contentTop = cardY + 56;
        const int viewportH = cardH - 64;
        const int totalContentH = 295;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        const int scrollY = std::clamp(engine.getPauseScrollY(), 0, maxScroll);

        // Click on Scrollbar Track
        if (maxScroll > 0 && clickPos.x() >= cardX + cardW - 16 && clickPos.x() <= cardX + cardW - 2
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const float ratio = static_cast<float>(clickPos.y() - contentTop) / viewportH;
            engine.setPauseScrollY(static_cast<int>(ratio * maxScroll));
            redrawPixels();
            return;
        }

        if (clickPos.x() >= cardX + 4 && clickPos.x() <= cardX + cardW - 16
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const QPoint logicalPos(clickPos.x(), clickPos.y() + scrollY);

            // Difficulty Mode pills
            const int modeBtnW = std::min(104, (cardW - 60) / 3);
            const int modeBtnH = 26;
            const int modeGap = 8;
            const int totalModeW = modeBtnW * 3 + modeGap * 2;
            const int startModeX = cardX + (cardW - totalModeW) / 2;
            const int modeBarY = contentTop + 6;

            const QRect easyRect(startModeX, modeBarY, modeBtnW, modeBtnH);
            const QRect medRect(startModeX + modeBtnW + modeGap, modeBarY, modeBtnW, modeBtnH);
            const QRect hardRect(startModeX + (modeBtnW + modeGap) * 2, modeBarY, modeBtnW, modeBtnH);

            if (easyRect.contains(logicalPos))
            {
                engine.setMode(GameMode::EASY, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            else if (medRect.contains(logicalPos))
            {
                engine.setMode(GameMode::MEDIUM, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            else if (hardRect.contains(logicalPos))
            {
                engine.setMode(GameMode::HARD, myGrid);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }

            // Action Buttons
            const int btnW = cardW - 70;
            const int btnH = 34;
            const int btnGap = 8;
            const int btnX = cardX + (cardW - btnW) / 2;
            const int startY = modeBarY + 50;

            const QRect btnResume(btnX, startY, btnW, btnH);
            const QRect btnSettings(btnX, startY + (btnH + btnGap), btnW, btnH);
            const QRect btnRestart(btnX, startY + (btnH + btnGap) * 2, btnW, btnH);
            const QRect btnMainMenu(btnX, startY + (btnH + btnGap) * 3, btnW, btnH);
            const QRect btnExit(btnX, startY + (btnH + btnGap) * 4, btnW, btnH);

            if (btnResume.contains(logicalPos))
            {
                togglePause();
                return;
            }
            else if (btnSettings.contains(logicalPos))
            {
                openSettings();
                return;
            }
            else if (btnRestart.contains(logicalPos))
            {
                resetGame();
                return;
            }
            else if (btnMainMenu.contains(logicalPos))
            {
                returnToMainMenu();
                return;
            }
            else if (btnExit.contains(logicalPos))
            {
                audioManager.playSound(SoundId::UI_CLICK);
                QApplication::quit();
                return;
            }
        }

        // Clicking anywhere inside the card frame should not resume accidentally
        const QRect cardRect(cardX, cardY, cardW, cardH);
        if (cardRect.contains(clickPos))
        {
            return;
        }

        // Clicking outside card resumes
        togglePause();
        return;
    }

    // 4. SETTINGS STATE
    if (engine.getState() == GameState::SETTINGS)
    {
        // 4.1 Check small Back button OUTSIDE the page frame
        const QRect btnBack = GameRenderer::getSettingsBackButtonRect(width, height);
        if (btnBack.contains(clickPos))
        {
            engine.setState(previousStateBeforeSettings);
            if (previousStateBeforeSettings == GameState::PLAYING)
            {
                audioManager.resumeBGM();
                lockCursorToFrame();
            }
            audioManager.playSound(SoundId::UI_CLICK);
            updateHUD();
            redrawPixels();
            return;
        }

        const int cardW = std::min(width - 40, 560);
        const int cardH = std::min(height - 40, 350);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        const int contentTop = cardY + 52;
        const int viewportH = cardH - 60;
        const int totalContentH = 325;
        const int maxScroll = std::max(0, totalContentH - viewportH);
        const int scrollY = std::clamp(engine.getSettingsScrollY(), 0, maxScroll);

        // Click on Scrollbar Track
        if (maxScroll > 0 && clickPos.x() >= cardX + cardW - 16 && clickPos.x() <= cardX + cardW - 2
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const float ratio = static_cast<float>(clickPos.y() - contentTop) / viewportH;
            engine.setSettingsScrollY(static_cast<int>(ratio * maxScroll));
            redrawPixels();
            return;
        }

        if (clickPos.x() >= cardX + 4 && clickPos.x() <= cardX + cardW - 16
            && clickPos.y() >= contentTop && clickPos.y() <= contentTop + viewportH)
        {
            const QPoint logicalPos(clickPos.x(), clickPos.y() + scrollY);
            const int rowLeftX = cardX + 30;

            // --- ROW 1: Music (BGM) ---
            const int y1 = contentTop + 8;
            const QRect bgmMuteBtn(rowLeftX + 120, y1, 60, 26);
            if (bgmMuteBtn.contains(logicalPos))
            {
                audioManager.setBGMMuted(!audioManager.isBGMMuted());
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }

            const int volStartX = rowLeftX + 190;
            const int volBtnW = 48;
            const int volBtnH = 26;
            const int volGap = 6;
            const float volValues[5] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
            for (int i = 0; i < 5; ++i)
            {
                const QRect vb(volStartX + i * (volBtnW + volGap), y1, volBtnW, volBtnH);
                if (vb.contains(logicalPos))
                {
                    if (audioManager.isBGMMuted())
                    {
                        audioManager.setBGMMuted(false);
                    }
                    audioManager.setBGMVolume(volValues[i]);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
            }

            // --- ROW 2: Sound Effects (SFX) ---
            const int y2 = contentTop + 48;
            const QRect sfxMuteBtn(rowLeftX + 120, y2, 60, 26);
            if (sfxMuteBtn.contains(logicalPos))
            {
                audioManager.setSFXMuted(!audioManager.isSFXMuted());
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            for (int i = 0; i < 5; ++i)
            {
                const QRect vb(volStartX + i * (volBtnW + volGap), y2, volBtnW, volBtnH);
                if (vb.contains(logicalPos))
                {
                    if (audioManager.isSFXMuted())
                    {
                        audioManager.setSFXMuted(false);
                    }
                    audioManager.setSFXVolume(volValues[i]);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
            }

            // --- ROW 3: Controls Mode ---
            const int y3 = contentTop + 88;
            const int ctlStartX = rowLeftX + 110;
            const int ctlBtnW = 82;
            const int ctlBtnH = 26;
            const int ctlGap = 6;
            const ControlMode modes[4] = {
                ControlMode::SCREEN_TOUCH,
                ControlMode::VIRTUAL_BUTTONS,
                ControlMode::GYROSCOPE,
                ControlMode::MOUSE_KEYBOARD
            };
            for (int i = 0; i < 4; ++i)
            {
                const QRect cb(ctlStartX + i * (ctlBtnW + ctlGap), y3, ctlBtnW, ctlBtnH);
                if (cb.contains(logicalPos))
                {
                    engine.setControlMode(modes[i]);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
            }

            // --- ROW 4: Thunder Flash ---
            const int y4 = contentTop + 128;
            const QRect btnFlashOn(rowLeftX + 120, y4, 105, 26);
            const QRect btnFlashOff(rowLeftX + 235, y4, 150, 26);
            if (btnFlashOn.contains(logicalPos))
            {
                engine.setLightningFlashEnabled(true);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
            else if (btnFlashOff.contains(logicalPos))
            {
                engine.setLightningFlashEnabled(false);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }

            // --- ROW 5: Pixel Scale ---
            const int y5 = contentTop + 168;
            const int scales[4] = {3, 4, 5, 6};
            for (int i = 0; i < 4; ++i)
            {
                const QRect sb(rowLeftX + 120 + i * 62, y5, 52, 26);
                if (sb.contains(logicalPos))
                {
                    scale = scales[i];
                    myGrid.setScale(scale);
                    audioManager.playSound(SoundId::UI_CLICK);
                    redrawPixels();
                    return;
                }
            }
        }

        // Clicking anywhere inside the card frame outside buttons does nothing
        const QRect cardRect(cardX, cardY, cardW, cardH);
        if (cardRect.contains(clickPos))
        {
            return;
        }

        // Clicking outside card frame closes settings
        engine.setState(previousStateBeforeSettings);
        if (previousStateBeforeSettings == GameState::PLAYING)
        {
            audioManager.resumeBGM();
            lockCursorToFrame();
        }
        audioManager.playSound(SoundId::UI_CLICK);
        updateHUD();
        redrawPixels();
        return;
    }

    // 5. PLAYING STATE
    // 5.1 Pause Button
    const QRect pauseRect = GameRenderer::getPauseButtonRect(width, height);
    if (pauseRect.contains(clickPos))
    {
        audioManager.playSound(SoundId::UI_CLICK);
        togglePause();
        return;
    }

    // 5.2 Growth Egg Button [ 🧺 H ]
    if (engine.getGrowthEggCount() > 0)
    {
        const QRect hRect = GameRenderer::getGrowthButtonRect(width, height);
        if (hRect.contains(clickPos))
        {
            engine.useGrowthEgg();
            redrawPixels();
            return;
        }
    }

    // 5.3 Companion Basket Button [ 🤖 C ]
    if (engine.getMode() == GameMode::HARD && engine.getGrowthEggCount() >= 2)
    {
        const QRect cRect = GameRenderer::getCompanionButtonRect(width, height);
        if (cRect.contains(clickPos))
        {
            engine.summonCompanionBasket();
            redrawPixels();
            return;
        }
    }

    // 5.4 Mode-specific actions
    if (engine.getControlMode() == ControlMode::VIRTUAL_BUTTONS)
    {
        const QRect leftRect = GameRenderer::getVirtualLeftButtonRect(width, height);
        const QRect rightRect = GameRenderer::getVirtualRightButtonRect(width, height);
        if (leftRect.contains(clickPos))
        {
            engine.setKeyLeft(true);
            engine.setKeyRight(false);
            return;
        }
        else if (rightRect.contains(clickPos))
        {
            engine.setKeyRight(true);
            engine.setKeyLeft(false);
            return;
        }
        return;
    }
    else if (engine.getControlMode() == ControlMode::GYROSCOPE)
    {
        return;
    }

    engine.handleMouseLeftClick(clickPos.x());
    updateHUD();
}

void MainWindow::onMouseLeftReleased()
{
    if (engine.getControlMode() == ControlMode::VIRTUAL_BUTTONS)
    {
        engine.setKeyLeft(false);
        engine.setKeyRight(false);
    }
}

void MainWindow::onMouseRightClicked()
{
    togglePause();
}
