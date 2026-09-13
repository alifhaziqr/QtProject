#include "mainwindow.h"

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTextDocument>

#include <cmath>
#include <functional>

class PolygonHandle;

class CanvasPolygon : public QGraphicsPolygonItem
{
public:
    explicit CanvasPolygon(const QPolygonF &points): QGraphicsPolygonItem(points)
    {
        setBrush(QColor("#58b7a8"));
        setPen(QPen(QColor("#000000"), 3));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setCursor(Qt::OpenHandCursor);
    }

    void setChangedCallback(std::function<void()> callback) { changed = std::move(callback); }
    void addHandle(PolygonHandle *handle) { handles.append(handle); }
    void addPointLabel(int index);
    void movePoint(int index, const QPointF &scenePosition);
    QString pointLabelText(int index) const;
    void setPointLabelText(int index, const QString &text);
    QColor pointLabelColor(int index) const;
    void setPointLabelColor(int index, const QColor &color);
    QColor fillColor() const { return brush().color(); }

    void setFillColor(const QColor &color)
    {
        setBrush(color);
        if (changed)
        {
            changed();
        }
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        const QVariant result = QGraphicsPolygonItem::itemChange(change, value);
        if (change == QGraphicsItem::ItemPositionHasChanged && changed)
        {
            changed();
        }
        return result;
    }

private:
    QList<PolygonHandle *> handles;
    QList<QGraphicsTextItem *> pointLabels;
    std::function<void()> changed;
};

class PolygonHandle : public QGraphicsEllipseItem
{
public:
    PolygonHandle(int index, CanvasPolygon *owner): QGraphicsEllipseItem(-6, -6, 12, 12, owner), index(index), owner(owner)
    {
        setBrush(QColor("#f8fafc"));
        setPen(QPen(QColor("#2563eb"), 2));
        setFlag(QGraphicsItem::ItemIsMovable);
        setCursor(Qt::OpenHandCursor);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        setCursor(Qt::ClosedHandCursor);
        QGraphicsEllipseItem::mousePressEvent(event);
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton)
            owner->movePoint(index, event->scenePos());
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        setCursor(Qt::OpenHandCursor);
        owner->movePoint(index, event->scenePos());
        QGraphicsEllipseItem::mouseReleaseEvent(event);
    }

private:
    int index;
    CanvasPolygon *owner;
};

void CanvasPolygon::movePoint(int index, const QPointF &scenePosition)
{
    if (index < 0 || index >= handles.size()) return;
    
    const QPointF localPosition = mapFromScene(scenePosition);
    QPolygonF points = polygon();
    points[index] = localPosition;
    setPolygon(points);
    handles[index]->setPos(localPosition);

    if (index < pointLabels.size())
    {
        pointLabels[index]->setPos(localPosition + QPointF(9, -22));
    }

    if (changed)
    {
        changed();
    }
}

void CanvasPolygon::addPointLabel(int index)
{
    auto *pointLabel = new QGraphicsTextItem(QString("P%1").arg(index + 1), this);
    pointLabel->setTextInteractionFlags(Qt::TextEditorInteraction);
    pointLabel->setDefaultTextColor(QColor("#164e63"));
    pointLabel->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
    pointLabel->setPos(polygon().at(index) + QPointF(9, -22));
    QObject::connect(pointLabel->document(), &QTextDocument::contentsChanged,[this] { if (changed) changed(); });
    pointLabels.append(pointLabel);
}

QString CanvasPolygon::pointLabelText(int index) const
{
    if (index < 0 || index >= pointLabels.size())
        return {};

    return pointLabels.at(index)->toPlainText();
}

void CanvasPolygon::setPointLabelText(int index, const QString &text)
{
    if (index < 0 || index >= pointLabels.size() || pointLabelText(index) == text) return;

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
    if (index < 0 || index >= pointLabels.size()) return;
        
    pointLabels[index]->setDefaultTextColor(color);
    if (changed) 
    {   
        changed();
    }
}

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle("Polygon Editor");
    resize(1050, 680);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(-360, -260, 720, 520);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor("#ffffff"));
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setMinimumWidth(600);

    const QPolygonF points{{-170, -110}, {30, -160}, {190, -45}, {120, 145}, {-120, 165}};
    polygon = new CanvasPolygon(points);
    scene->addItem(polygon);

    for (int i = 0; i < points.size(); ++i) {
        auto *handle = new PolygonHandle(i, polygon);
        handle->setPos(points.at(i));
        polygon->addHandle(handle);
        polygon->addPointLabel(i);
    }

    polygon->setChangedCallback([this] { updateProperties(); });

    auto *properties = new QGroupBox("Properties", this);
    auto *propertiesLayout = new QGridLayout(properties);
    fillButton = new QPushButton("Choose color", properties);
    connect(fillButton, &QPushButton::clicked, this, &MainWindow::chooseFillColor);
    propertiesLayout->addWidget(new QLabel("Fill", properties), 0, 0);
    propertiesLayout->addWidget(fillButton, 0, 1);

    areaLabel = new QLabel(properties);
    propertiesLayout->addWidget(new QLabel("Area", properties), 1, 0);
    propertiesLayout->addWidget(areaLabel, 1, 1);

    for (int i = 0; i < 5; ++i) {
        auto *pointBox = new QGroupBox(QString("Point %1").arg(i + 1), properties);
        auto *pointLayout = new QGridLayout(pointBox);

        for (int axis = 0; axis < 2; ++axis) {
            auto *field = new QDoubleSpinBox(pointBox);
            field->setRange(-1000.0, 1000.0);
            field->setDecimals(1);
            field->setSingleStep(1.0);
            pointFields[i][axis] = field;
            connect(field, &QDoubleSpinBox::valueChanged, this,[this, i] { updatePoint(i, pointFields[i][0]->value(), pointFields[i][1]->value()); });
            pointLayout->addWidget(new QLabel(axis == 0 ? "X" : "Y", pointBox), axis, 0);
            pointLayout->addWidget(field, axis, 1);
        }

        auto *labelField = new QLineEdit(pointBox);
        pointLabelFields[i] = labelField;
        connect(labelField, &QLineEdit::textChanged, this,[this, i](const QString &text) { polygon->setPointLabelText(i, text); });
        pointLayout->addWidget(new QLabel("Label", pointBox), 2, 0);
        pointLayout->addWidget(labelField, 2, 1);

        auto *labelColorButton = new QPushButton("Choose color", pointBox);
        pointLabelColorButtons[i] = labelColorButton;
        connect(labelColorButton, &QPushButton::clicked, this,[this, i] { choosePointLabelColor(i); });
        pointLayout->addWidget(new QLabel("Text color", pointBox), 3, 0);
        pointLayout->addWidget(labelColorButton, 3, 1);
        propertiesLayout->addWidget(pointBox, i + 2, 0, 1, 2);
    }

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(view);
    splitter->addWidget(properties);
    splitter->setStretchFactor(0, 1);
    splitter->setSizes({760, 290});
    setCentralWidget(splitter);
    updateProperties();
}

void MainWindow::updateProperties()
{
    const QPolygonF points = polygon->polygon();
    for (int i = 0; i < points.size() && i < 5; ++i) {
        const QPointF scenePoint = polygon->mapToScene(points.at(i));
        const QSignalBlocker xBlocker(pointFields[i][0]);
        const QSignalBlocker yBlocker(pointFields[i][1]);
        pointFields[i][0]->setValue(scenePoint.x());
        pointFields[i][1]->setValue(scenePoint.y());
        const QSignalBlocker labelBlocker(pointLabelFields[i]);
        pointLabelFields[i]->setText(polygon->pointLabelText(i));
        const QColor labelColor = polygon->pointLabelColor(i);
        pointLabelColorButtons[i]->setStyleSheet(QString("background-color: %1; color: %2;").arg(labelColor.name(),labelColor.lightness() < 140 ? "white" : "#16324f"));
    }

    double twiceArea = 0.0;
    for (int i = 0; i < points.size(); ++i) {
        const QPointF &current = points.at(i);
        const QPointF &next = points.at((i + 1) % points.size());
        twiceArea += current.x() * next.y() - next.x() * current.y();
    }

    areaLabel->setText(QString::number(std::abs(twiceArea) / 2.0, 'f', 1) + " px2");

    const QColor color = polygon->fillColor();
    fillButton->setStyleSheet(QString("background-color: %1; color: %2;").arg(color.name(),
        color.lightness() < 140 ? "white" : "#16324f"));

}

void MainWindow::updatePoint(int index, double x, double y)
{
    if (!polygon || index < 0 || index >= polygon->polygon().size()) return;

    polygon->movePoint(index, QPointF(x, y));
}

void MainWindow::chooseFillColor()
{
    const QColor color = QColorDialog::getColor(polygon->fillColor(), this, "Polygon fill color");
    if (color.isValid())
    {
        polygon->setFillColor(color);
    }
}

void MainWindow::choosePointLabelColor(int index)
{
    const QColor color = QColorDialog::getColor(polygon->pointLabelColor(index), this,QString("Point %1 text color").arg(index + 1));
    if (color.isValid())
    {
        polygon->setPointLabelColor(index, color);
    }
}
