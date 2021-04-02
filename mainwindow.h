#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QToolButton>
#include "k4adevice.h"
#include "devmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QButtonGroup *syncModeButtons[N_CAM];
    devManager *devs;
    QVector<QLabel*> colorLabels;
    QVector<QLabel*> depthLabels;
    QVector<QLabel*> whitebalanceLabels;
    QVector<QPushButton*> devOpenButtons;
    QVector<QPushButton*> camStartButtons;
    QVector<QSpinBox*> exposureSpinBoxes;
    QVector<QSlider*> whitebalanceSliders;
    QVector<QToolButton*> exposureAutoButtons;
    QVector<QToolButton*> whitebalanceAutoButtons;

private slots:
    void slotGetColorImg(QImage);
    void slotGetDepthImg(QImage);

    void devOpenButtons_clicked();
    void camStartButtons_clicked();
    void exposureSpinBoxes_valueChanged(int arg1);
    void whitebalanceSliders_sliderMoved(int position);
    void exposureAutoButtons_clicked();
    void whitebalanceAutoButtons_clicked();
};

#endif // MAINWINDOW_H
