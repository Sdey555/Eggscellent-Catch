#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QKeyEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include "Grid.h"
#include "Pixel.h"
#include "GameEngine.h"
#include "GameRenderer.h"
#include "AudioManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void gameLoopTick();
    void onGridPanned(int dx, int dy);

    // Mouse & Touch controls
    void onMouseMoved(QPoint &pos);
    void onMouseLeftClicked();
    void onMouseLeftReleased();
    void onMouseRightClicked();
    void onDragScrolled(int dy);
    void onWheelScrolled(int delta);
    void startGame();

private:
    Ui::MainWindow *ui;
    QTimer *gameTimer = nullptr;

    int scale = 4;
    int hudTick = 0;
    int menuSlideIndex = 0;

    MyGrid myGrid;
    MyPixels myPixels;
    GameEngine engine;
    GameRenderer renderer;
    AudioManager audioManager;

    void lockCursorToFrame();
    void unlockCursor();

    void resetGame();
    void togglePause();
    void returnToMainMenu();
    void openSettings();
    void updateHUD();
    void redrawPixels();
    void nextMenuSlide();
    void prevMenuSlide();

    GameState previousStateBeforeSettings{GameState::PAUSED};
};

#endif // MAINWINDOW_H
