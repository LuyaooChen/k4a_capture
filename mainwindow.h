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
};

#endif // MAINWINDOW_H
