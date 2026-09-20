#ifndef CANVASPOLYGON_H
#define CANVASPOLYGON_H

#include <QGraphicsPolygonItem>

class PolygonHandle;
class QGraphicsTextItem;

class CanvasPolygon : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT

public:
    explicit CanvasPolygon(const QPolygonF &points);

    void addHandle(PolygonHandle *handle);
    void addPointLabel(int index);
    void movePoint(int index, const QPointF &scenePosition);
    QString pointLabelText(int index) const;
    void setPointLabelText(int index, const QString &text);
    QColor pointLabelColor(int index) const;
    void setPointLabelColor(int index, const QColor &color);
    QColor fillColor() const;
    void setFillColor(const QColor &color);

signals:
    void polygonChanged();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QList<PolygonHandle *> handles;
    QList<QGraphicsTextItem *> pointLabels;
};

#endif // CANVASPOLYGON_H
