#include "mainwindow.h"

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGroupBox>
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
    explicit CanvasPolygon(const QPolygonF &points)
        : QGraphicsPolygonItem(points)
    {
        setBrush(QColor("#58b7a8"));
        setPen(QPen(QColor("#164e63"), 3));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        setCursor(Qt::OpenHandCursor);
    }

    void setChangedCallback(std::function<void()> callback) { changed = std::move(callback); }
    void addHandle(PolygonHandle *handle) { handles.append(handle); }
    void movePoint(int index, const QPointF &scenePosition);
    QColor fillColor() const { return brush().color(); }

    void setFillColor(const QColor &color)
    {
        setBrush(color);
        if (changed)
            changed();
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override
    {
        const QVariant result = QGraphicsPolygonItem::itemChange(change, value);
        if (change == QGraphicsItem::ItemPositionHasChanged && changed)
            changed();
        return result;
    }

private:
    QList<PolygonHandle *> handles;
    std::function<void()> changed;
};

class PolygonHandle : public QGraphicsEllipseItem
{
public:
    PolygonHandle(int index, CanvasPolygon *owner)
        : QGraphicsEllipseItem(-6, -6, 12, 12, owner), index(index), owner(owner)
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
    if (index < 0 || index >= handles.size())
        return;
    const QPointF localPosition = mapFromScene(scenePosition);
    QPolygonF points = polygon();
    points[index] = localPosition;
    setPolygon(points);
    handles[index]->setPos(localPosition);
    if (changed)
        changed();
}

class EditableLabel : public QGraphicsTextItem
{
public:
    explicit EditableLabel(const QString &text)
        : QGraphicsTextItem(text)
    {
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setDefaultTextColor(QColor("#16324f"));
        setFont(QFont("Segoe UI", 14, QFont::DemiBold));
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsSelectable);
    }

    void setChangedCallback(std::function<void()> callback)
    {
        changed = std::move(callback);
        QObject::connect(document(), &QTextDocument::contentsChanged,
                         [this] { if (changed) changed(); });
    }

private:
    std::function<void()> changed;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Polygon Canvas");
    resize(1050, 680);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(-360, -260, 720, 520);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor("#eef5f4"));
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setMinimumWidth(600);

    const QPolygonF points{{-170, -110}, {30, -160}, {190, -45}, {120, 145}, {-120, 165}};
    polygon = new CanvasPolygon(points);
    scene->addItem(polygon);
    for (int i = 0; i < points.size(); ++i) {
        auto *handle = new PolygonHandle(i, polygon);
        handle->setPos(points.at(i));
        polygon->addHandle(handle);
    }
    polygon->setChangedCallback([this] { updateProperties(); });

    label = new EditableLabel("Editable label");
    label->setPos(-65, 205);
    scene->addItem(label);
    static_cast<EditableLabel *>(label)->setChangedCallback([this] { updateProperties(); });

    auto *properties = new QGroupBox("Properties", this);
    auto *form = new QFormLayout(properties);
    fillButton = new QPushButton("Choose color", properties);
    connect(fillButton, &QPushButton::clicked, this, &MainWindow::chooseFillColor);
    form->addRow("Fill", fillButton);

    areaLabel = new QLabel(properties);
    form->addRow("Area", areaLabel);

    for (int i = 0; i < 5; ++i) {
        auto *pointBox = new QGroupBox(QString("Point %1").arg(i + 1), properties);
        auto *pointForm = new QFormLayout(pointBox);
        for (int axis = 0; axis < 2; ++axis) {
            auto *field = new QDoubleSpinBox(pointBox);
            field->setRange(-1000.0, 1000.0);
            field->setDecimals(1);
            field->setSingleStep(1.0);
            pointFields[i][axis] = field;
            connect(field, &QDoubleSpinBox::valueChanged, this,
                    [this, i] { updatePoint(i, pointFields[i][0]->value(), pointFields[i][1]->value()); });
            pointForm->addRow(axis == 0 ? "X" : "Y", field);
        }
        form->addRow(pointBox);
    }

    textEdit = new QLineEdit(properties);
    connect(textEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (label->toPlainText() != text)
            label->setPlainText(text);
    });
    form->addRow("Label text", textEdit);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(view);
    splitter->addWidget(properties);
    splitter->setStretchFactor(0, 1);
    splitter->setSizes({760, 290});
    setCentralWidget(splitter);
    updateProperties();
}

MainWindow::~MainWindow() = default;

void MainWindow::updateProperties()
{
    const QPolygonF points = polygon->polygon();
    for (int i = 0; i < points.size() && i < 5; ++i) {
        const QPointF scenePoint = polygon->mapToScene(points.at(i));
        const QSignalBlocker xBlocker(pointFields[i][0]);
        const QSignalBlocker yBlocker(pointFields[i][1]);
        pointFields[i][0]->setValue(scenePoint.x());
        pointFields[i][1]->setValue(scenePoint.y());
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
    const QSignalBlocker textBlocker(textEdit);
    textEdit->setText(label->toPlainText());
}

void MainWindow::updatePoint(int index, double x, double y)
{
    if (!polygon || index < 0 || index >= polygon->polygon().size())
        return;
    polygon->movePoint(index, QPointF(x, y));
}

void MainWindow::chooseFillColor()
{
    const QColor color = QColorDialog::getColor(polygon->fillColor(), this, "Polygon fill color");
    if (color.isValid())
        polygon->setFillColor(color);
}
