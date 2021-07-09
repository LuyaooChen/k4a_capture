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
#include <open3d/visualization/visualizer/Visualizer.h>
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
    QVector<QLabel*> colorLabels;
    QVector<QLabel*> depthLabels;
    QVector<QLabel*> maskLabels;
    QVector<QLabel*> whitebalanceLabels;
    QVector<QPushButton*> devOpenButtons;
    QVector<QPushButton*> camStartButtons;
    QVector<QSpinBox*> exposureSpinBoxes;
    QVector<QSlider*> whitebalanceSliders;
    QVector<QToolButton*> exposureAutoButtons;
    QVector<QToolButton*> whitebalanceAutoButtons;

    devManager *devs;
    std::shared_ptr<open3d::visualization::Visualizer> pc_viewer;

private Q_SLOT:
    void slotGetColorImg(QImage);
    void slotGetDepthImg(QImage);
    void slotGetMaskImg(QImage);
    void slotPointCloudReady(bool);
    void slotFPSUpdate(float);

    void devOpenButtons_clicked();
    void camStartButtons_clicked();
    void exposureSpinBoxes_valueChanged(int arg1);
    void whitebalanceSliders_sliderMoved(int position);
    void exposureAutoButtons_clicked();
    void whitebalanceAutoButtons_clicked();
    void on_visualModeAction_triggered();
    void on_refineRegistrationAction_triggered();
    void on_saveImgsAction_triggered();
    void on_startAllAction_triggered();
    void on_setBGAction_triggered();
    void on_savePCAction_triggered();
    void on_recordAction_triggered();
};

#endif // MAINWINDOW_H
