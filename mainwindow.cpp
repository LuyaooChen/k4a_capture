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
    devs->setVisualMode(VISUALIZATION_MODE_2D);
    for(int i=0;i<N_CAM;i++)
    {
        devs->k4aDevices[i]->setObjectName(QString::number(i));   //简单用数字命名设备对象
        connect(devs->k4aDevices[i],SIGNAL(sig_SendColorImg(QImage)),this,SLOT(slotGetColorImg(QImage)));
        connect(devs->k4aDevices[i],SIGNAL(sig_SendDepthImg(QImage)),this,SLOT(slotGetDepthImg(QImage)));
    }
    connect(devs,SIGNAL(sig_SendPointCloudReady(bool)),this,SLOT(slotPointCloudReady(bool)));
    connect(devs,SIGNAL(sig_FPS(float)),this,SLOT(slotFPSUpdate(float)));

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
    /* 最大值最小值默认值enable等直接在ui界面中设置了，这里就省的写了。
        // ...
    */
    }
    devs->begin();
}

MainWindow::~MainWindow()
{
    for(int i=0;i<N_CAM;i++)
        delete syncModeButtons[i];
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
        camStartButtons[index]->setEnabled(false);
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
        camStartButtons[index]->setEnabled(true);
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

void MainWindow::on_visualModeAction_triggered()
{
    if(devs->getVisualMode()==VISUALIZATION_MODE_2D)
    {
        pc_viewer = std::shared_ptr<open3d::visualization::Visualizer>(new open3d::visualization::Visualizer());
        pc_viewer->CreateVisualizerWindow("Cloud",1280,720);
        devs->setVisualMode(VISUALIZATION_MODE_3D);
        qDebug()<<"3d on";
//        pc_viewer->PrintVisualizerHelp();
    }
    else
    {
        devs->setVisualMode(VISUALIZATION_MODE_2D);
        pc_viewer->ClearGeometries();
        pc_viewer->DestroyVisualizerWindow();
        devs->_is_viewerOpened=false;
        qDebug()<<"2d on";
    }
}

void MainWindow::slotPointCloudReady(bool flag)
{
    if(flag)
    {
        devs->mutex.lock();
        if(!devs->_is_viewerOpened)
        {
            for(int i=0;i<N_CAM;i++)
                pc_viewer->AddGeometry(devs->pointcloud[i]);
            devs->_is_viewerOpened=true;
        }
        else
        {
            pc_viewer->UpdateGeometry();
            pc_viewer->PollEvents();
            pc_viewer->UpdateRender();
        }
        devs->mutex.unlock();
    }
}

void MainWindow::on_refineRegistrationAction_triggered()
{
    devs->refineRegistration_on=true;
}

void MainWindow::on_saveImgsAction_triggered()
{
    devs->saveAllImgs_on=true;
}

void MainWindow::on_startAllAction_triggered()
{
    uint32_t N_devs = k4a::device::get_installed_count();
    if(ui->startAllAction->text()=="Start All")
    {
        int master_index=-1;
        for(int index=0;index<N_devs;index++)
        {
            if(!devs->k4aDevices[index]->is_opened())
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
                camStartButtons[index]->setEnabled(true);
                for(int i=0;i<3;i++)
                    syncModeButtons[index]->button(i)->setEnabled(false);

                // start camera.
                if(sync_mode!=K4A_WIRED_SYNC_MODE_MASTER)
                {
                    devs->k4aDevices[index]->startCamera();
                    camStartButtons[index]->setText("Stop");
                }
                else master_index=index;
            }
        }
        // MASTER设备最后启动
        if(master_index!=-1)
        {
            devs->k4aDevices[master_index]->startCamera();
            camStartButtons[master_index]->setText("Stop");
        }
        ui->startAllAction->setText("Close All");
    }
    else
    {
        for(int index=0;index<N_devs;index++)
        {
            if(devs->k4aDevices[index]->is_opened())
            {
                devs->k4aDevices[index]->close();

                devOpenButtons[index]->setText("Open Device");
                camStartButtons[index]->setText("Start");
                exposureSpinBoxes[index]->setEnabled(false);
                whitebalanceSliders[index]->setEnabled(false);
                exposureAutoButtons[index]->setEnabled(false);
                whitebalanceAutoButtons[index]->setEnabled(false);
                camStartButtons[index]->setEnabled(false);
                for(int i=0;i<3;i++)
                    syncModeButtons[index]->button(i)->setEnabled(true);
            }
        }
        ui->startAllAction->setText("Start All");
    }
}

void MainWindow::slotFPSUpdate(float fps)
{
    ui->statusBar->showMessage(QString("FPS:")+QString::number(double(fps),'f',1));
}
