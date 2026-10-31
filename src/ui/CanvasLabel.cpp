#include "CanvasLabel.h"
#include <algorithm>

CanvasLabel::CanvasLabel(QWidget *parent)
    : QLabel(parent)
{
    this->setMouseTracking(true);
    this->setCursor(Qt::CrossCursor);
}

void CanvasLabel::mouseMoveEvent(QMouseEvent *ev)
{
    const int clampedX = std::clamp(ev->pos().x(), 0, std::max(1, this->width() - 1));
    QPoint clampedPos(clampedX, ev->pos().y());
    emit sendMousePosition(clampedPos);

    if (isLeftPressed)
    {
        const int dy = ev->pos().y() - lastDragY;
        lastDragY = ev->pos().y();

        if ((ev->pos() - dragStartPos).manhattanLength() > 14)
        {
            hasDragged = true;
        }

        if (dy != 0 && hasDragged)
        {
            emit dragScrolled(dy);
        }
    }

    if (isPanning)
    {
        int dx = ev->pos().x() - lastDragPos.x();
        int dy = ev->pos().y() - lastDragPos.y();
        lastDragPos = ev->pos();
        if (dx != 0 || dy != 0)
        {
            emit panDelta(dx, dy);
        }
    }
    else if (isRightPressed)
    {
        // Right button: a short click toggles pause, a drag pans the view
        if ((ev->pos() - dragStartPos).manhattanLength() > 4)
        {
            isPanning = true;
            setCursor(Qt::ClosedHandCursor);
            int dx = ev->pos().x() - lastDragPos.x();
            int dy = ev->pos().y() - lastDragPos.y();
            lastDragPos = ev->pos();
            if (dx != 0 || dy != 0)
            {
                emit panDelta(dx, dy);
            }
        }
    }
}

void CanvasLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        isLeftPressed = true;
        const int clampedX = std::clamp(ev->pos().x(), 0, std::max(1, this->width() - 1));
        dragStartPos = QPoint(clampedX, ev->pos().y());
        lastDragPos = dragStartPos;
        lastDragY = ev->pos().y();
        hasDragged = false;
        lastClick = dragStartPos;

        // Immediately notify mouse position on press (for gameplay basket / virtual button response)
        emit sendMousePosition(lastClick);
    }
    else if (ev->button() == Qt::RightButton)
    {
        isRightPressed = true;
        isPanning = false;
        dragStartPos = ev->pos();
        lastDragPos = ev->pos();
    }
    else if (ev->button() == Qt::MiddleButton)
    {
        isPanning = true;
        lastDragPos = ev->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void CanvasLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        isLeftPressed = false;
        emit leftReleased();

        // If the user did not drag/swipe, trigger tap/click
        if (!hasDragged)
        {
            const int clampedX = std::clamp(ev->pos().x(), 0, std::max(1, this->width() - 1));
            lastClick = QPoint(clampedX, ev->pos().y());
            emit Mouse_Pos();
        }
    }
    else if (ev->button() == Qt::RightButton)
    {
        if (isRightPressed && !isPanning)
        {
            emit rightClicked();
        }
        isRightPressed = false;
        isPanning = false;
        setCursor(Qt::CrossCursor);
    }
    else if (ev->button() == Qt::MiddleButton)
    {
        isPanning = false;
        setCursor(Qt::CrossCursor);
    }
}

void CanvasLabel::wheelEvent(QWheelEvent *ev)
{
    const int dy = ev->angleDelta().y();
    if (dy != 0)
    {
        emit wheelScrolled(dy);
    }
    QLabel::wheelEvent(ev);
}
