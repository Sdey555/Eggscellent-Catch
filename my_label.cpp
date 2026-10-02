#include "my_label.h"

my_label::my_label(QWidget *parent) : QLabel(parent)
{
    this->setMouseTracking(true);
}

void my_label::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint pos = ev->pos();
    if (pos.x() >= 0 && pos.y() >= 0 && pos.x() < this->width() && pos.y() < this->height()) {
        emit sendMousePosition(pos);
    }

    if (isPanning) {
        int dx = ev->pos().x() - lastDragPos.x();
        int dy = ev->pos().y() - lastDragPos.y();
        lastDragPos = ev->pos();
        if (dx != 0 || dy != 0) {
            emit panDelta(dx, dy);
        }
    } else if (isLeftPressed) {
        if ((ev->pos() - dragStartPos).manhattanLength() > 4) {
            isPanning = true;
            setCursor(Qt::ClosedHandCursor);
            int dx = ev->pos().x() - lastDragPos.x();
            int dy = ev->pos().y() - lastDragPos.y();
            lastDragPos = ev->pos();
            if (dx != 0 || dy != 0) {
                emit panDelta(dx, dy);
            }
        }
    }
}

void my_label::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        isLeftPressed = true;
        isPanning = false;
        dragStartPos = ev->pos();
        lastDragPos = ev->pos();
    } else if (ev->button() == Qt::RightButton || ev->button() == Qt::MiddleButton) {
        isPanning = true;
        lastDragPos = ev->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void my_label::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        if (isLeftPressed && !isPanning) {
            lastClick = ev->pos();
            emit Mouse_Pos();
        }
        isLeftPressed = false;
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    } else if (ev->button() == Qt::RightButton || ev->button() == Qt::MiddleButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}