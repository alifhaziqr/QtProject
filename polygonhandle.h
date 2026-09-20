#ifndef POLYGONHANDLE_H
#define POLYGONHANDLE_H

#include <QGraphicsEllipseItem>

class CanvasPolygon;
class QGraphicsSceneMouseEvent;

class PolygonHandle : public QGraphicsEllipseItem
{
public:
    PolygonHandle(int index, CanvasPolygon *owner);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    int index;
    CanvasPolygon *owner;
};

#endif // POLYGONHANDLE_H
