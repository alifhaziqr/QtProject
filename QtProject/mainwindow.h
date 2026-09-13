#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CanvasPolygon;
class QGraphicsScene;
class QGraphicsView;
class QGraphicsTextItem;
class QLineEdit;
class QPushButton;
class QLabel;
class QDoubleSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void updateProperties();
    void updatePoint(int index, double x, double y);
    void chooseFillColor();

    QGraphicsScene *scene = nullptr;
    QGraphicsView *view = nullptr;
    CanvasPolygon *polygon = nullptr;
    QGraphicsTextItem *label = nullptr;
    QPushButton *fillButton = nullptr;
    QLabel *areaLabel = nullptr;
    QLineEdit *textEdit = nullptr;
    QDoubleSpinBox *pointFields[5][2]{};
};
#endif // MAINWINDOW_H
