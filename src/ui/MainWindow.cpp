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
    engine.setState(GameState::MENU);
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
    else if (engine.getState() == GameState::SETTINGS)
    {
        if (key == Qt::Key_Escape || key == Qt::Key_Backspace)
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

void MainWindow::onMouseLeftClicked()
{
    const int width = ui->frame->width();
    const int height = ui->frame->height();
    const QPoint clickPos = ui->frame->lastClickPosition();

    // 1. MENU STATE
    if (engine.getState() == GameState::MENU)
    {
        const int cardW = std::min(width - 40, 520);
        const int cardH = std::min(height - 40, 390);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;
        const int modeBtnW = 100;
        const int modeBtnH = 26;
        const int modeBarY = cardY + 75;
        const int startModeX = cardX + (cardW - (modeBtnW * 3 + 20)) / 2;

        // Check Settings Button on Menu Card
        const QRect menuSettingsBtn(cardX + cardW - 105, cardY + 16, 85, 26);
        if (menuSettingsBtn.contains(clickPos))
        {
            openSettings();
            return;
        }

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
        const int cardH = std::min(height - 40, 360);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;

        // Difficulty Mode pills
        const int modeBtnW = std::min(104, (cardW - 60) / 3);
        const int modeBtnH = 27;
        const int modeGap = 8;
        const int totalModeW = modeBtnW * 3 + modeGap * 2;
        const int startModeX = cardX + (cardW - totalModeW) / 2;
        const int modeBarY = cardY + 74;

        const QRect easyRect(startModeX, modeBarY, modeBtnW, modeBtnH);
        const QRect medRect(startModeX + modeBtnW + modeGap, modeBarY, modeBtnW, modeBtnH);
        const QRect hardRect(startModeX + (modeBtnW + modeGap) * 2, modeBarY, modeBtnW, modeBtnH);

        if (easyRect.contains(clickPos))
        {
            engine.setMode(GameMode::EASY, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (medRect.contains(clickPos))
        {
            engine.setMode(GameMode::MEDIUM, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (hardRect.contains(clickPos))
        {
            engine.setMode(GameMode::HARD, myGrid);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }

        const int btnW = cardW - 70;
        const int btnH = 36;
        const int btnGap = 9;
        const int btnX = cardX + (cardW - btnW) / 2;
        const int startY = cardY + 130;

        const QRect btnResume(btnX, startY, btnW, btnH);
        const QRect btnSettings(btnX, startY + (btnH + btnGap), btnW, btnH);
        const QRect btnRestart(btnX, startY + (btnH + btnGap) * 2, btnW, btnH);
        const QRect btnMainMenu(btnX, startY + (btnH + btnGap) * 3, btnW, btnH);

        if (btnResume.contains(clickPos))
        {
            togglePause();
            return;
        }
        else if (btnSettings.contains(clickPos))
        {
            openSettings();
            return;
        }
        else if (btnRestart.contains(clickPos))
        {
            resetGame();
            return;
        }
        else if (btnMainMenu.contains(clickPos))
        {
            returnToMainMenu();
            return;
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
        const int cardW = std::min(width - 40, 560);
        const int cardH = std::min(height - 40, 480);
        const int cardX = (width - cardW) / 2;
        const int cardY = (height - cardH) / 2;
        const int rowLeftX = cardX + 35;

        const int volStartX = rowLeftX + 205;
        const int volBtnW = 50;
        const int volBtnH = 26;
        const int volGap = 7;
        const float volValues[5] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};

        // --- ROW 1: Music (BGM) ---
        const int y1 = cardY + 76;
        const QRect bgmMuteBtn(rowLeftX + 130, y1, 65, 26);
        if (bgmMuteBtn.contains(clickPos))
        {
            audioManager.setBGMMuted(!audioManager.isBGMMuted());
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        for (int i = 0; i < 5; ++i)
        {
            const QRect vb(volStartX + i * (volBtnW + volGap), y1, volBtnW, volBtnH);
            if (vb.contains(clickPos))
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
        const int y2 = cardY + 124;
        const QRect sfxMuteBtn(rowLeftX + 130, y2, 65, 26);
        if (sfxMuteBtn.contains(clickPos))
        {
            audioManager.setSFXMuted(!audioManager.isSFXMuted());
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        for (int i = 0; i < 5; ++i)
        {
            const QRect vb(volStartX + i * (volBtnW + volGap), y2, volBtnW, volBtnH);
            if (vb.contains(clickPos))
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
        const int y3 = cardY + 172;
        const int ctlStartX = rowLeftX + 115;
        const int ctlBtnW = 86;
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
            if (cb.contains(clickPos))
            {
                engine.setControlMode(modes[i]);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
        }

        // --- ROW 4: Thunder Flash ---
        const int y4 = cardY + 220;
        const QRect btnFlashOn(rowLeftX + 130, y4, 110, 26);
        const QRect btnFlashOff(rowLeftX + 250, y4, 160, 26);
        if (btnFlashOn.contains(clickPos))
        {
            engine.setLightningFlashEnabled(true);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }
        else if (btnFlashOff.contains(clickPos))
        {
            engine.setLightningFlashEnabled(false);
            audioManager.playSound(SoundId::UI_CLICK);
            redrawPixels();
            return;
        }

        // --- ROW 5: Pixel Scale ---
        const int y5 = cardY + 268;
        const int scales[4] = {3, 4, 5, 6};
        for (int i = 0; i < 4; ++i)
        {
            const QRect sb(rowLeftX + 130 + i * 65, y5, 55, 26);
            if (sb.contains(clickPos))
            {
                scale = scales[i];
                myGrid.setScale(scale);
                audioManager.playSound(SoundId::UI_CLICK);
                redrawPixels();
                return;
            }
        }

        // --- Back / Done Button ---
        const int backW = 190;
        const int backH = 36;
        const int backX = cardX + (cardW - backW) / 2;
        const int backY = cardY + cardH - 50;
        const QRect btnBack(backX, backY, backW, backH);
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
