#ifndef CANVASLABEL_H
#define CANVASLABEL_H

#include <QLabel>
#include <QMouseEvent>

class CanvasLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CanvasLabel(QWidget *parent = nullptr);
    QPoint lastClickPosition() const { return lastClick; }

protected:
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

signals:
    void sendMousePosition(QPoint &pos);
    void Mouse_Pos();
    void rightClicked();
    void panDelta(int dx, int dy);

private:
    QPoint lastClick;
    QPoint dragStartPos;
    QPoint lastDragPos;
    bool isLeftPressed{false};
    bool isRightPressed{false};
    bool isPanning{false};
};

#endif // CANVASLABEL_H
