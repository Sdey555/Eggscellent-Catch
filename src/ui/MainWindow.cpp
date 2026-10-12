#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "CanvasLabel.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QCursor>
#include <algorithm>

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

void MainWindow::nextMenuSlide()
{
    menuSlideIndex = (menuSlideIndex + 1) % 3;
    engine.setMenuSlideIndex(menuSlideIndex);
    redrawPixels();
}

void MainWindow::startGame()
{
    engine.setState(GameState::PLAYING);

    // Fix cursor at middle of the game canvas and hide it
    const QPoint center = ui->frame->mapToGlobal(QPoint(ui->frame->width() / 2, ui->frame->height() / 2));
    QCursor::setPos(center);
    ui->frame->setCursor(Qt::BlankCursor);

    updateHUD();
    redrawPixels();
}

void MainWindow::resetGame()
{
    myGrid.setDimensions(ui->frame->width(), ui->frame->height());
    engine.reset(myGrid);

    // Fix cursor at middle of the game canvas and hide it
    const QPoint center = ui->frame->mapToGlobal(QPoint(ui->frame->width() / 2, ui->frame->height() / 2));
    QCursor::setPos(center);
    ui->frame->setCursor(Qt::BlankCursor);

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
    if (engine.getState() == GameState::PAUSED)
    {
        ui->frame->setCursor(Qt::ArrowCursor);
    }
    else if (engine.getState() == GameState::PLAYING)
    {
        ui->frame->setCursor(Qt::BlankCursor);
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

    if (engine.getState() == GameState::MENU)
    {
        if (key == Qt::Key_Space || key == Qt::Key_Return || key == Qt::Key_Enter)
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
    if (engine.getState() == GameState::PLAYING || engine.getState() == GameState::MENU)
    {
        engine.tick(myGrid);
        hudTick++;
    }
    updateHUD();
    redrawPixels();
}

void MainWindow::updateHUD()
{
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
        startGame();
        return;
    }
    if (engine.getState() == GameState::GAME_OVER)
    {
        resetGame();
        return;
    }
    engine.handleMouseLeftClick(ui->frame->lastClickPosition().x());
    if (engine.getState() == GameState::PLAYING)
    {
        ui->frame->setCursor(Qt::BlankCursor);
    }
    updateHUD();
    redrawPixels();
}

void MainWindow::onMouseRightClicked()
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
    togglePause();
    redrawPixels();
}
