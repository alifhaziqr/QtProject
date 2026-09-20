#include "polygonhandle.h"

#include "canvaspolygon.h"

#include <QBrush>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPen>

PolygonHandle::PolygonHandle(int index, CanvasPolygon *owner)
    : QGraphicsEllipseItem(-6, -6, 12, 12, owner), index(index), owner(owner)
{
    setBrush(QColor("#f8fafc"));
    setPen(QPen(QColor("#2563eb"), 2));
    setFlag(QGraphicsItem::ItemIsMovable);
    setCursor(Qt::OpenHandCursor);
}

void PolygonHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::ClosedHandCursor);
    QGraphicsEllipseItem::mousePressEvent(event);
}

void PolygonHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        owner->movePoint(index, event->scenePos());
}

void PolygonHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    owner->movePoint(index, event->scenePos());
    QGraphicsEllipseItem::mouseReleaseEvent(event);
}
