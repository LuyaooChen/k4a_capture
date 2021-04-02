#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPixmap>
#include <QString>
#include <QValidator>

#include <QDebug>

#define DEFAULT_EXPOSURE_EXP -8
#define DEFAULT_WHITEBALANCE 4500

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    devs = new devManager();
    for(int i=0;i<N_CAM;i++)
    {
        devs->k4aDevices[i]->setObjectName(QString::number(i));   //简单用数字命名设备对象
        connect(devs->k4aDevices[i],SIGNAL(sig_SendColorImg(QImage)),this,SLOT(slotGetColorImg(QImage)));
        connect(devs->k4aDevices[i],SIGNAL(sig_SendDepthImg(QImage)),this,SLOT(slotGetDepthImg(QImage)));
    }

    {
    // 同步模式设置按钮组
    syncModeButtons[0] = new QButtonGroup(this);
    syncModeButtons[0]->addButton(ui->standaloneRadio_0,K4A_WIRED_SYNC_MODE_STANDALONE);
    syncModeButtons[0]->addButton(ui->masterRadio_0,K4A_WIRED_SYNC_MODE_MASTER);
    syncModeButtons[0]->addButton(ui->subRadio_0,K4A_WIRED_SYNC_MODE_SUBORDINATE);
    ui->standaloneRadio_0->setChecked(true);

    syncModeButtons[1] = new QButtonGroup(this);
    syncModeButtons[1]->addButton(ui->standaloneRadio_1,K4A_WIRED_SYNC_MODE_STANDALONE);
    syncModeButtons[1]->addButton(ui->masterRadio_1,K4A_WIRED_SYNC_MODE_MASTER);
    syncModeButtons[1]->addButton(ui->subRadio_1,K4A_WIRED_SYNC_MODE_SUBORDINATE);
    ui->subRadio_1->setChecked(true);

    syncModeButtons[2] = new QButtonGroup(this);
    syncModeButtons[2]->addButton(ui->standaloneRadio_2,K4A_WIRED_SYNC_MODE_STANDALONE);
    syncModeButtons[2]->addButton(ui->masterRadio_2,K4A_WIRED_SYNC_MODE_MASTER);
    syncModeButtons[2]->addButton(ui->subRadio_2,K4A_WIRED_SYNC_MODE_SUBORDINATE);
    ui->subRadio_2->setChecked(true);

    syncModeButtons[3] = new QButtonGroup(this);
    syncModeButtons[3]->addButton(ui->standaloneRadio_3,K4A_WIRED_SYNC_MODE_STANDALONE);
    syncModeButtons[3]->addButton(ui->masterRadio_3,K4A_WIRED_SYNC_MODE_MASTER);
    syncModeButtons[3]->addButton(ui->subRadio_3,K4A_WIRED_SYNC_MODE_SUBORDINATE);
    ui->subRadio_3->setChecked(true);
    }

    {
    //各类控件分组，方便后续操作
    colorLabels.append(ui->colorLabel_0);
    colorLabels.append(ui->colorLabel_1);
    colorLabels.append(ui->colorLabel_2);
    colorLabels.append(ui->colorLabel_3);

    depthLabels.append(ui->depthLabel_0);
    depthLabels.append(ui->depthLabel_1);
    depthLabels.append(ui->depthLabel_2);
    depthLabels.append(ui->depthLabel_3);

    whitebalanceLabels.append(ui->whitebalanceLabel_0);
    whitebalanceLabels.append(ui->whitebalanceLabel_1);
    whitebalanceLabels.append(ui->whitebalanceLabel_2);
    whitebalanceLabels.append(ui->whitebalanceLabel_3);

    devOpenButtons.append(ui->devOpenButton_0);
    devOpenButtons.append(ui->devOpenButton_1);
    devOpenButtons.append(ui->devOpenButton_2);
    devOpenButtons.append(ui->devOpenButton_3);

    camStartButtons.append(ui->camStartButton_0);
    camStartButtons.append(ui->camStartButton_1);
    camStartButtons.append(ui->camStartButton_2);
    camStartButtons.append(ui->camStartButton_3);

    exposureSpinBoxes.append(ui->exposureSpinBox_0);
    exposureSpinBoxes.append(ui->exposureSpinBox_1);
    exposureSpinBoxes.append(ui->exposureSpinBox_2);
    exposureSpinBoxes.append(ui->exposureSpinBox_3);

    whitebalanceSliders.append(ui->whitebalanceSlider_0);
    whitebalanceSliders.append(ui->whitebalanceSlider_1);
    whitebalanceSliders.append(ui->whitebalanceSlider_2);
    whitebalanceSliders.append(ui->whitebalanceSlider_3);

    exposureAutoButtons.append(ui->exposureAutoButton_0);
    exposureAutoButtons.append(ui->exposureAutoButton_1);
    exposureAutoButtons.append(ui->exposureAutoButton_2);
    exposureAutoButtons.append(ui->exposureAutoButton_3);

    whitebalanceAutoButtons.append(ui->whitebalanceAutoButton_0);
    whitebalanceAutoButtons.append(ui->whitebalanceAutoButton_1);
    whitebalanceAutoButtons.append(ui->whitebalanceAutoButton_2);
    whitebalanceAutoButtons.append(ui->whitebalanceAutoButton_3);
    }

    for(int i=0;i<N_CAM;i++)
    {
        // 相同功能的按钮连接到同一个槽
        connect(devOpenButtons[i],SIGNAL(clicked(bool)),this,SLOT(devOpenButtons_clicked()));
        connect(camStartButtons[i],SIGNAL(clicked(bool)),this,SLOT(camStartButtons_clicked()));
        connect(exposureSpinBoxes[i],SIGNAL(valueChanged(int)),this,SLOT(exposureSpinBoxes_valueChanged(int)));
        connect(whitebalanceSliders[i],SIGNAL(sliderMoved(int)),this,SLOT(whitebalanceSliders_sliderMoved(int)));
        connect(exposureAutoButtons[i],SIGNAL(clicked(bool)),this,SLOT(exposureAutoButtons_clicked()));
        connect(whitebalanceAutoButtons[i],SIGNAL(clicked(bool)),this,SLOT(whitebalanceAutoButtons_clicked()));
    }

    {
    // 颜色控制设置
    /* 最大值最小值默认值enable等直接在ui界面中设置了，这里就省的写了。
    ui->exposureSpinBox_0->setMinimum(-11);
    ui->exposureSpinBox_0->setMaximum(1);
    ui->exposureSpinBox_0->setValue(DEFAULT_EXPOSURE_EXP);
    ui->exposureSpinBox_0->setEnabled(false);

    ui->whitebalanceSlider_0->setMaximum(7000);   //超出[2566,12500]这个范围会抛出异常
    ui->whitebalanceSlider_0->setMinimum(2600);
    ui->whitebalanceSlider_0->setValue(DEFAULT_WHITEBALANCE);
    ui->whitebalanceLabel_0->setText(QString::number(DEFAULT_WHITEBALANCE));
    ui->whitebalanceSlider_0->setEnabled(false);
    */
    }

    devs->begin();
}

MainWindow::~MainWindow()
{
    delete devs;
    delete ui;
}

void MainWindow::slotGetColorImg(QImage img)
{
    int index=sender()->objectName().toInt();
    colorLabels[index]->setPixmap(QPixmap::fromImage(img.scaled(colorLabels[index]->size(),Qt::KeepAspectRatio)));
}

void MainWindow::slotGetDepthImg(QImage img)
{
    int index=sender()->objectName().toInt();
    depthLabels[index]->setPixmap(QPixmap::fromImage(img.scaled(depthLabels[index]->size(),Qt::KeepAspectRatio)));
}

void MainWindow::devOpenButtons_clicked()
{
    int index = sender()->objectName().split("_").back().toInt();
    if(devs->k4aDevices[index]->is_opened())
    {
        devs->k4aDevices[index]->close();

        devOpenButtons[index]->setText("Open Device");
        camStartButtons[index]->setText("Start");
        exposureSpinBoxes[index]->setEnabled(false);
        whitebalanceSliders[index]->setEnabled(false);
        exposureAutoButtons[index]->setEnabled(false);
        whitebalanceAutoButtons[index]->setEnabled(false);
        for(int i=0;i<3;i++)
            syncModeButtons[index]->button(i)->setEnabled(true);
    }
    else
    {
        k4a_wired_sync_mode_t sync_mode = (k4a_wired_sync_mode_t)(syncModeButtons[index]->checkedId());
        devs->k4aDevices[index]->setSyncMode(sync_mode);
        if(!devs->k4aDevices[index]->open())
        {
            QMessageBox::information(NULL,"ERROR!","Fail to open device "+QString::number(index));
            return;
        }
        devs->k4aDevices[index]->setExposureTime(exposureSpinBoxes[index]->value());
        devs->k4aDevices[index]->setWhiteBalance(whitebalanceSliders[index]->value());

        /* ui settings */
        devOpenButtons[index]->setText("Close Device");
        exposureSpinBoxes[index]->setEnabled(true);
        whitebalanceSliders[index]->setEnabled(true);
        exposureAutoButtons[index]->setEnabled(true);
        whitebalanceAutoButtons[index]->setEnabled(true);
        for(int i=0;i<3;i++)
            syncModeButtons[index]->button(i)->setEnabled(false);
    }
}

void MainWindow::camStartButtons_clicked()
{
    int index = sender()->objectName().split("_").back().toInt();
    if(devs->k4aDevices[index]->is_camRunning())
    {
        devs->k4aDevices[index]->stopCamera();
        camStartButtons[index]->setText("Start");
    }
    else
    {
        devs->k4aDevices[index]->startCamera();
        camStartButtons[index]->setText("Stop");
    }
}

void MainWindow::exposureSpinBoxes_valueChanged(int arg1)
{
    int index = sender()->objectName().split("_").back().toInt();
    devs->k4aDevices[index]->setExposureTime(arg1);
}

void MainWindow::whitebalanceSliders_sliderMoved(int position)
{
    int index = sender()->objectName().split("_").back().toInt();
    devs->k4aDevices[index]->setWhiteBalance(position);
    whitebalanceLabels[index]->setText(QString::number(position));
}

void MainWindow::exposureAutoButtons_clicked()
{
    int index = sender()->objectName().split("_").back().toInt();
    if(exposureAutoButtons[index]->text()=="M")
    {
        // 自动模式下似乎会从设定值开始自动调整
        devs->k4aDevices[index]->setExposureTime(DEFAULT_EXPOSURE_EXP,K4A_COLOR_CONTROL_MODE_AUTO);
        exposureAutoButtons[index]->setText("A");
        exposureSpinBoxes[index]->setEnabled(false);
    }
    else
    {
        devs->k4aDevices[index]->setExposureTime(exposureSpinBoxes[index]->value());
        exposureAutoButtons[index]->setText("M");
        exposureSpinBoxes[index]->setEnabled(true);
    }
}

void MainWindow::whitebalanceAutoButtons_clicked()
{
    int index = sender()->objectName().split("_").back().toInt();
    if(whitebalanceAutoButtons[index]->text()=="M")
    {
        devs->k4aDevices[index]->setWhiteBalance(DEFAULT_WHITEBALANCE,K4A_COLOR_CONTROL_MODE_AUTO);
        whitebalanceAutoButtons[index]->setText("A");
        whitebalanceSliders[index]->setEnabled(false);
    }
    else
    {
        devs->k4aDevices[index]->setWhiteBalance(whitebalanceSliders[index]->value());
        whitebalanceAutoButtons[index]->setText("M");
        whitebalanceSliders[index]->setEnabled(true);
    }
}



