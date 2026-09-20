#include "canvaspolygon.h"

#include "polygonhandle.h"

#include <QBrush>
#include <QCursor>
#include <QPen>
#include <QGraphicsTextItem>
#include <QTextDocument>

CanvasPolygon::CanvasPolygon(const QPolygonF &points)
    : QObject(), QGraphicsPolygonItem(points)
{
    setBrush(QColor("#58b7a8"));
    setPen(QPen(QColor("#000000"), 3));
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    setCursor(Qt::OpenHandCursor);
}

void CanvasPolygon::addHandle(PolygonHandle *handle)
{
    handles.append(handle);
}

void CanvasPolygon::addPointLabel(int index)
{
    auto *pointLabel = new QGraphicsTextItem(QString("P%1").arg(index + 1), this);
    pointLabel->setTextInteractionFlags(Qt::TextEditorInteraction);
    pointLabel->setDefaultTextColor(QColor("#164e63"));
    pointLabel->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    pointLabel->setPos(polygon().at(index) + QPointF(9, -22));
    QObject::connect(pointLabel->document(), &QTextDocument::contentsChanged,
                     this, &CanvasPolygon::polygonChanged);
    pointLabels.append(pointLabel);
}

void CanvasPolygon::movePoint(int index, const QPointF &scenePosition)
{
    if (index < 0 || index >= handles.size())
        return;

    const QPointF localPosition = mapFromScene(scenePosition);
    QPolygonF points = polygon();
    points[index] = localPosition;
    setPolygon(points);
    handles[index]->setPos(localPosition);

    if (index < pointLabels.size())
        pointLabels[index]->setPos(localPosition + QPointF(9, -22));

    emit polygonChanged();
}

QString CanvasPolygon::pointLabelText(int index) const
{
    if (index < 0 || index >= pointLabels.size())
        return {};

    return pointLabels.at(index)->toPlainText();
}

void CanvasPolygon::setPointLabelText(int index, const QString &text)
{
    if (index < 0 || index >= pointLabels.size() || pointLabelText(index) == text)
        return;

    pointLabels[index]->setPlainText(text);
}

QColor CanvasPolygon::pointLabelColor(int index) const
{
    if (index < 0 || index >= pointLabels.size())
        return QColor("#164e63");

    return pointLabels.at(index)->defaultTextColor();
}

void CanvasPolygon::setPointLabelColor(int index, const QColor &color)
{
    if (index < 0 || index >= pointLabels.size())
        return;

    pointLabels[index]->setDefaultTextColor(color);
    emit polygonChanged();
}

QColor CanvasPolygon::fillColor() const
{
    return brush().color();
}

void CanvasPolygon::setFillColor(const QColor &color)
{
    setBrush(color);
    emit polygonChanged();
}

QVariant CanvasPolygon::itemChange(GraphicsItemChange change, const QVariant &value)
{
    const QVariant result = QGraphicsPolygonItem::itemChange(change, value);
    if (change == QGraphicsItem::ItemPositionHasChanged)
        emit polygonChanged();

    return result;
}
