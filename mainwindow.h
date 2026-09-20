#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CanvasPolygon;
class QGraphicsScene;
class QGraphicsView;
class QLineEdit;
class QPushButton;
class QLabel;
class QDoubleSpinBox;
class QGridLayout;
class QGroupBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    static constexpr int PointCount = 5;

    void setupWindow();
    void setupCanvas();
    void setupPropertiesPanel();
    void createPointEditor(int index, QGroupBox *properties, QGridLayout *layout);
    void updateProperties();
    void updatePoint(int index, double x, double y);
    void updatePointLabel(int index, const QString &text);
    void chooseFillColor();
    void choosePointLabelColor(int index);

    QGraphicsScene *scene = nullptr;
    QGraphicsView *view = nullptr;
    CanvasPolygon *polygon = nullptr;
    QPushButton *fillButton = nullptr;
    QLabel *areaLabel = nullptr;
    QDoubleSpinBox *pointFields[PointCount][2]{};
    QLineEdit *pointLabelFields[PointCount]{};
    QPushButton *pointLabelColorButtons[PointCount]{};
};
#endif // MAINWINDOW_H
