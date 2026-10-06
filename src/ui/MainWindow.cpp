#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "CanvasLabel.h"
#include <QTimer>
#include <QLabel>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Eggscellent Catch");

    ui->spinScale->setMinimum(2);
    ui->spinScale->setValue(4);
    scale = 4;
    myGrid.setScale(scale);

    this->installEventFilter(this);

    connect(ui->frame, &CanvasLabel::panDelta, this, &MainWindow::onGridPanned);
    connect(ui->frame, &CanvasLabel::sendMousePosition, this, &MainWindow::onMouseMoved);
    connect(ui->frame, &CanvasLabel::Mouse_Pos, this, &MainWindow::onMouseLeftClicked);
    connect(ui->frame, &CanvasLabel::rightClicked, this, &MainWindow::onMouseRightClicked);

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

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoopTick);
    gameTimer->start(30);

    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    engine.init(myGrid);
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

void MainWindow::resetGame()
{
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    engine.reset(myGrid);
    ui->btnPause->setText("Pause");
    updateHUD();
    redrawPixels();
}

void MainWindow::togglePause()
{
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    engine.togglePause();
    if (engine.getState() == GameState::PAUSED)
    {
        ui->btnPause->setText("Resume");
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        ui->btnPause->setText("Pause");
    }
    updateHUD();
    redrawPixels();
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
        engine.setKeyLeft(true);
    }
    else if (key == Qt::Key_Right || key == Qt::Key_D)
    {
        engine.setKeyRight(true);
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
    if (engine.getState() == GameState::PLAYING)
    {
        engine.tick(myGrid);
        hudTick++;
    }
    updateHUD();
    redrawPixels();
}

void MainWindow::updateHUD()
{
    ui->lblScore->setText(QString("SCORE: %1").arg(engine.getScore()));
    ui->lblHighScore->setText(QString("BEST: %1").arg(engine.getHighScore()));

    QString heartsText;
    for (int i = 0; i < 3; ++i)
    {
        heartsText += (i < engine.getHearts()) ? "❤" : "♡";
    }
    ui->lblHearts->setText(heartsText);

    if (lblLevel)
    {
        lblLevel->setText(engine.getLevel() >= GameEngine::MAX_LEVEL
                              ? QString("LEVEL %1 (MAX)").arg(engine.getLevel())
                              : QString("LEVEL %1").arg(engine.getLevel()));
    }

    if (!engine.getStatusMessage().isEmpty())
    {
        ui->lblStatus->setText(engine.getStatusMessage());
    }
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

void MainWindow::onMouseMoved(QPoint &pos)
{
    engine.handleMouseMove(pos.x());
}

void MainWindow::onMouseLeftClicked()
{
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    engine.handleMouseLeftClick(ui->frame->lastClickPosition().x());
    if (engine.getState() == GameState::PLAYING)
    {
        ui->btnPause->setText("Pause");
    }
    updateHUD();
    redrawPixels();
}

void MainWindow::onMouseRightClicked()
{
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    togglePause();
    redrawPixels();
}
