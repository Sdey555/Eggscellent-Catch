#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QKeyEvent>
#include <QResizeEvent>
#include "Grid.h"
#include "Pixel.h"
#include "GameEngine.h"
#include "GameRenderer.h"

namespace Ui {
class MainWindow;
}

class QLabel;
class QTimer;

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
    void onGridPanned(int dx, int dy);

    // Mouse controls
    void onMouseMoved(QPoint &pos);
    void onMouseLeftClicked();
    void onMouseRightClicked();
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

    void resetGame();
    void togglePause();
    void updateHUD();
    void redrawPixels();
    void nextMenuSlide();
};

#endif // MAINWINDOW_H
