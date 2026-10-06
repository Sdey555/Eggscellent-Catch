#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QMouseEvent>

class my_label : public QLabel
{
    Q_OBJECT
public:
    explicit my_label(QWidget *parent = nullptr);
    QPoint lastClickPosition() const { return lastClick; }

protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

signals:
    void sendMousePosition(QPoint&);
    void Mouse_Pos();
    void rightClicked();
    void panDelta(int dx, int dy);

private:
    QPoint lastClick;
    QPoint dragStartPos;
    QPoint lastDragPos;
    bool isLeftPressed = false;
    bool isRightPressed = false;
    bool isPanning = false;
};

#endif // MY_LABEL_H
