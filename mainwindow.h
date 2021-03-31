#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include "k4adevice.h"

#define N_CAM 4

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
    k4aDevice* k4aDevices[N_CAM];

    QButtonGroup* syncModeButtons[N_CAM];

private slots:
    void slotGetColorImg(QImage);
    void slotGetDepthImg(QImage);
    void on_devOpenButton_0_clicked();
    void on_camStartButton_0_clicked();
    void on_devOpenButton_1_clicked();
    void on_camStartButton_1_clicked();
    void on_devOpenButton_2_clicked();
    void on_camStartButton_2_clicked();
    void on_devOpenButton_3_clicked();
    void on_camStartButton_3_clicked();
    void on_exposureSpinBox_0_valueChanged(int arg1);
    void on_whitebalanceSlider_0_sliderMoved(int position);
    void on_exposureSpinBox_1_valueChanged(int arg1);
    void on_whitebalanceSlider_1_sliderMoved(int position);
    void on_exposureSpinBox_2_valueChanged(int arg1);
    void on_whitebalanceSlider_2_sliderMoved(int position);
    void on_exposureSpinBox_3_valueChanged(int arg1);
    void on_whitebalanceSlider_3_sliderMoved(int position);
    void on_exposureAutoButton_0_clicked();
    void on_whitebalanceAutoButton_0_clicked();
    void on_exposureAutoButton_1_clicked();
    void on_whitebalanceAutoButton_1_clicked();
    void on_exposureAutoButton_2_clicked();
    void on_whitebalanceAutoButton_2_clicked();
    void on_exposureAutoButton_3_clicked();
    void on_whitebalanceAutoButton_3_clicked();
};

#endif // MAINWINDOW_H
