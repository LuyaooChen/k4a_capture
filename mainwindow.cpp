#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPixmap>
#include <QString>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for(int i=0;i<N_CAM;i++)
    {
        k4aDevices[i] = new k4aDevice(i);
        k4aDevices[i]->setObjectName(QString::number(i));   //简单用数字命名设备对象
        connect(k4aDevices[i],SIGNAL(sig_SendColorImg(QImage)),this,SLOT(slotGetColorImg(QImage)));
        connect(k4aDevices[i],SIGNAL(sig_SendDepthImg(QImage)),this,SLOT(slotGetDepthImg(QImage)));
    }

    {
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

}

MainWindow::~MainWindow()
{
    for(int i=0;i<N_CAM;i++)
        delete k4aDevices[i];
    delete ui;
}

void MainWindow::slotGetColorImg(QImage img)
{
    QObject *object = QObject::sender();
    k4aDevice *dev = qobject_cast<k4aDevice*>(object);
    int index=dev->objectName().toInt();

    switch(index)
    {
        case 0:ui->colorLabel_0->setPixmap(QPixmap::fromImage(img.scaled(ui->colorLabel_0->size(),Qt::KeepAspectRatio)));break;
        case 1:ui->colorLabel_1->setPixmap(QPixmap::fromImage(img.scaled(ui->colorLabel_1->size(),Qt::KeepAspectRatio)));break;
        case 2:ui->colorLabel_2->setPixmap(QPixmap::fromImage(img.scaled(ui->colorLabel_2->size(),Qt::KeepAspectRatio)));break;
        case 3:ui->colorLabel_3->setPixmap(QPixmap::fromImage(img.scaled(ui->colorLabel_3->size(),Qt::KeepAspectRatio)));break;
        default:
            throw std::logic_error("Invalid device index!");
    }

}

void MainWindow::slotGetDepthImg(QImage img)
{
    QObject *object = QObject::sender();
    k4aDevice *dev = qobject_cast<k4aDevice*>(object);
    int index=dev->objectName().toInt();

    switch(index)
    {
        case 0:ui->depthLabel_0->setPixmap(QPixmap::fromImage(img.scaled(ui->depthLabel_0->size(),Qt::KeepAspectRatio)));break;
        case 1:ui->depthLabel_1->setPixmap(QPixmap::fromImage(img.scaled(ui->depthLabel_1->size(),Qt::KeepAspectRatio)));break;
        case 2:ui->depthLabel_2->setPixmap(QPixmap::fromImage(img.scaled(ui->depthLabel_2->size(),Qt::KeepAspectRatio)));break;
        case 3:ui->depthLabel_3->setPixmap(QPixmap::fromImage(img.scaled(ui->depthLabel_3->size(),Qt::KeepAspectRatio)));break;
        default:
            throw std::logic_error("Invalid device index!");
    }
    ui->depthLabel_0->setPixmap(QPixmap::fromImage(img.scaled(ui->depthLabel_0->size(),Qt::KeepAspectRatio)));
}


void MainWindow::on_devOpenButton_0_clicked()
{
    if(k4aDevices[0]->is_opened())
    {
        k4aDevices[0]->close();
        ui->devOpenButton_0->setText("Open Device");
        ui->camStartButton_0->setText("Start");
    }
    else
    {
        k4a_wired_sync_mode_t sync_mode = (k4a_wired_sync_mode_t)(syncModeButtons[0]->checkedId());
        k4aDevices[0]->setSyncMode(sync_mode);
        if(!k4aDevices[0]->open())
        {
            QMessageBox::information(NULL,"ERROR!","Fail to open device 0.");
            return;
        }
        ui->devOpenButton_0->setText("Close Device");
    }
}

void MainWindow::on_camStartButton_0_clicked()
{
    if(k4aDevices[0]->is_camRunning())
    {
        k4aDevices[0]->stopCamera();
        ui->camStartButton_0->setText("Start");
    }
    else
    {
        k4aDevices[0]->startCamera();
        ui->camStartButton_0->setText("Stop");
    }
}

void MainWindow::on_devOpenButton_1_clicked()
{
    if(k4aDevices[1]->is_opened())
    {
        k4aDevices[1]->close();
        ui->devOpenButton_1->setText("Open Device");
        ui->camStartButton_1->setText("Start");
    }
    else
    {
        k4a_wired_sync_mode_t sync_mode = (k4a_wired_sync_mode_t)(syncModeButtons[1]->checkedId());
        k4aDevices[1]->setSyncMode(sync_mode);
        if(!k4aDevices[1]->open())
        {
            QMessageBox::information(NULL,"ERROR!","Fail to open device 1.");
            return;
        }
        ui->devOpenButton_1->setText("Close Device");
    }
}

void MainWindow::on_camStartButton_1_clicked()
{
    if(k4aDevices[1]->is_camRunning())
    {
        k4aDevices[1]->stopCamera();
        ui->camStartButton_1->setText("Start");
    }
    else
    {
        k4aDevices[1]->startCamera();
        ui->camStartButton_1->setText("Stop");
    }
}

void MainWindow::on_devOpenButton_2_clicked()
{
    if(k4aDevices[2]->is_opened())
    {
        k4aDevices[2]->close();
        ui->devOpenButton_2->setText("Open Device");
        ui->camStartButton_2->setText("Start");
    }
    else
    {
        k4a_wired_sync_mode_t sync_mode = (k4a_wired_sync_mode_t)(syncModeButtons[2]->checkedId());
        k4aDevices[2]->setSyncMode(sync_mode);
        if(!k4aDevices[2]->open())
        {
            QMessageBox::information(NULL,"ERROR!","Fail to open device 2.");
            return;
        }
        ui->devOpenButton_2->setText("Close Device");
    }
}

void MainWindow::on_camStartButton_2_clicked()
{
    if(k4aDevices[2]->is_camRunning())
    {
        k4aDevices[2]->stopCamera();
        ui->camStartButton_2->setText("Start");
    }
    else
    {
        k4aDevices[2]->startCamera();
        ui->camStartButton_2->setText("Stop");
    }
}

void MainWindow::on_devOpenButton_3_clicked()
{
    if(k4aDevices[3]->is_opened())
    {
        k4aDevices[3]->close();
        ui->devOpenButton_3->setText("Open Device");
        ui->camStartButton_3->setText("Start");
    }
    else
    {
        k4a_wired_sync_mode_t sync_mode = (k4a_wired_sync_mode_t)(syncModeButtons[3]->checkedId());
        k4aDevices[3]->setSyncMode(sync_mode);
        if(!k4aDevices[3]->open())
        {
            QMessageBox::information(NULL,"ERROR!","Fail to open device 3.");
            return;
        }
        ui->devOpenButton_3->setText("Close Device");
    }
}

void MainWindow::on_camStartButton_3_clicked()
{
    if(k4aDevices[3]->is_camRunning())
    {
        k4aDevices[3]->stopCamera();
        ui->camStartButton_3->setText("Start");
    }
    else
    {
        k4aDevices[3]->startCamera();
        ui->camStartButton_3->setText("Stop");
    }
}
