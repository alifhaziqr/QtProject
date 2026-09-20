#include "mainwindow.h"

#include "canvaspolygon.h"
#include "polygonhandle.h"

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSplitter>

#include <cmath>

namespace
{
double polygonArea(const QPolygonF &points)
{
    double twiceArea = 0.0;
    for (int i = 0; i < points.size(); ++i) {
        const QPointF &current = points.at(i);
        const QPointF &next = points.at((i + 1) % points.size());
        twiceArea += current.x() * next.y() - next.x() * current.y();
    }

    return std::abs(twiceArea) / 2.0;
}

void setColorButtonStyle(QPushButton *button, const QColor &color)
{
    const QString textColor = color.lightness() < 140
        ? QStringLiteral("white")
        : QStringLiteral("#16324f");
    button->setStyleSheet(QStringLiteral("background-color: %1; color: %2;")
                              .arg(color.name(), textColor));
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupWindow();
    setupCanvas();
    setupPropertiesPanel();
    updateProperties();
}

void MainWindow::setupWindow()
{
    setWindowTitle("Polygon Editor");
    resize(1050, 680);
}

void MainWindow::setupCanvas()
{
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

    connect(polygon, &CanvasPolygon::polygonChanged,
            this, &MainWindow::updateProperties);
}

void MainWindow::setupPropertiesPanel()
{
    auto *properties = new QGroupBox("Properties", this);
    auto *propertiesLayout = new QGridLayout(properties);
    fillButton = new QPushButton("Choose color", properties);
    connect(fillButton, &QPushButton::clicked, this, &MainWindow::chooseFillColor);
    propertiesLayout->addWidget(new QLabel("Fill", properties), 0, 0);
    propertiesLayout->addWidget(fillButton, 0, 1);

    areaLabel = new QLabel(properties);
    propertiesLayout->addWidget(new QLabel("Area", properties), 1, 0);
    propertiesLayout->addWidget(areaLabel, 1, 1);

    for (int i = 0; i < PointCount; ++i)
        createPointEditor(i, properties, propertiesLayout);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(view);
    splitter->addWidget(properties);
    splitter->setStretchFactor(0, 1);
    splitter->setSizes({760, 290});
    setCentralWidget(splitter);
}

void MainWindow::createPointEditor(int index, QGroupBox *properties, QGridLayout *layout)
{
    auto *pointBox = new QGroupBox(QString("Point %1").arg(index + 1), properties);
    auto *pointLayout = new QGridLayout(pointBox);

    for (int axis = 0; axis < 2; ++axis) {
        auto *field = new QDoubleSpinBox(pointBox);
        field->setRange(-1000.0, 1000.0);
        field->setDecimals(1);
        field->setSingleStep(1.0);
        pointFields[index][axis] = field;
        connect(field, &QDoubleSpinBox::valueChanged, this,
                [this, index] {
                    updatePoint(index, pointFields[index][0]->value(),
                                pointFields[index][1]->value());
                });
        pointLayout->addWidget(new QLabel(axis == 0 ? "X" : "Y", pointBox), axis, 0);
        pointLayout->addWidget(field, axis, 1);
    }

    auto *labelField = new QLineEdit(pointBox);
    pointLabelFields[index] = labelField;
    connect(labelField, &QLineEdit::textChanged, this,
            [this, index](const QString &text) {
                updatePointLabel(index, text);
            });
    pointLayout->addWidget(new QLabel("Label", pointBox), 2, 0);
    pointLayout->addWidget(labelField, 2, 1);

    auto *labelColorButton = new QPushButton("Choose color", pointBox);
    pointLabelColorButtons[index] = labelColorButton;
    connect(labelColorButton, &QPushButton::clicked, this,
            [this, index] { choosePointLabelColor(index); });
    pointLayout->addWidget(new QLabel("Text color", pointBox), 3, 0);
    pointLayout->addWidget(labelColorButton, 3, 1);
    layout->addWidget(pointBox, index + 2, 0, 1, 2);
}

void MainWindow::updateProperties()
{
    const QPolygonF points = polygon->polygon();
    for (int i = 0; i < points.size() && i < PointCount; ++i) {
        const QPointF scenePoint = polygon->mapToScene(points.at(i));
        const QSignalBlocker xBlocker(pointFields[i][0]);
        const QSignalBlocker yBlocker(pointFields[i][1]);
        pointFields[i][0]->setValue(scenePoint.x());
        pointFields[i][1]->setValue(scenePoint.y());
        const QSignalBlocker labelBlocker(pointLabelFields[i]);
        pointLabelFields[i]->setText(polygon->pointLabelText(i));
        const QColor labelColor = polygon->pointLabelColor(i);
        setColorButtonStyle(pointLabelColorButtons[i], labelColor);
    }

    areaLabel->setText(QString::number(polygonArea(points), 'f', 1) + " px2");

    const QColor color = polygon->fillColor();
    setColorButtonStyle(fillButton, color);

}

void MainWindow::updatePoint(int index, double x, double y)
{
    if (!polygon || index < 0 || index >= polygon->polygon().size()) return;

    polygon->movePoint(index, QPointF(x, y));
}

void MainWindow::updatePointLabel(int index, const QString &text)
{
    if (!polygon || index < 0 || index >= polygon->polygon().size())
        return;

    polygon->setPointLabelText(index, text);
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
