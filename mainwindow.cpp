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

    for(int i=0;i<N_CAM;i++)
    {
        k4aDevices[i] = new k4aDevice(0);
        k4aDevices[i]->setObjectName(QString::number(i));   //简单用数字命名设备对象
        connect(k4aDevices[i],SIGNAL(sig_SendColorImg(QImage)),this,SLOT(slotGetColorImg(QImage)));
        connect(k4aDevices[i],SIGNAL(sig_SendDepthImg(QImage)),this,SLOT(slotGetDepthImg(QImage)));
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
    // 颜色控制设置
    // 最大值最小值默认值enable等直接在ui界面中设置了，这里就省的写了。
//    ui->exposureSpinBox_0->setMinimum(-11);
//    ui->exposureSpinBox_0->setMaximum(1);
//    ui->exposureSpinBox_0->setValue(DEFAULT_EXPOSURE_EXP);
//    ui->exposureSpinBox_0->setEnabled(false);

//    ui->whitebalanceSlider_0->setMaximum(7000);   //超出[2566,12500]这个范围会抛出异常
//    ui->whitebalanceSlider_0->setMinimum(2600);
//    ui->whitebalanceSlider_0->setValue(DEFAULT_WHITEBALANCE);
//    ui->whitebalanceLabel_0->setText(QString::number(DEFAULT_WHITEBALANCE));
//    ui->whitebalanceSlider_0->setEnabled(false);


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
}


void MainWindow::on_devOpenButton_0_clicked()
{
    if(k4aDevices[0]->is_opened())
    {
        k4aDevices[0]->close();

        ui->devOpenButton_0->setText("Open Device");
        ui->camStartButton_0->setText("Start");
        ui->exposureSpinBox_0->setEnabled(false);
        ui->whitebalanceSlider_0->setEnabled(false);
        ui->exposureAutoButton_0->setEnabled(false);
        ui->whitebalanceAutoButton_0->setEnabled(false);
        for(int i=0;i<3;i++)
            syncModeButtons[0]->button(i)->setEnabled(true);
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
        k4aDevices[0]->setExposureTime(ui->exposureSpinBox_0->value());
        k4aDevices[0]->setWhiteBalance(ui->whitebalanceSlider_0->value());

        /* ui settings */
        ui->devOpenButton_0->setText("Close Device");
        ui->exposureSpinBox_0->setEnabled(true);
        ui->whitebalanceSlider_0->setEnabled(true);
        ui->exposureAutoButton_0->setEnabled(true);
        ui->whitebalanceAutoButton_0->setEnabled(true);
        for(int i=0;i<3;i++)
            syncModeButtons[0]->button(i)->setEnabled(false);
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
        ui->exposureSpinBox_1->setEnabled(false);
        ui->whitebalanceSlider_1->setEnabled(false);
        ui->exposureAutoButton_1->setEnabled(false);
        ui->whitebalanceAutoButton_1->setEnabled(false);
        for(int i=0;i<3;i++)
            syncModeButtons[1]->button(i)->setEnabled(true);
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
        k4aDevices[1]->setExposureTime(ui->exposureSpinBox_1->value());
        k4aDevices[1]->setWhiteBalance(ui->whitebalanceSlider_1->value());
        /* ui settings */
        ui->devOpenButton_1->setText("Close Device");
        ui->exposureSpinBox_1->setEnabled(true);
        ui->whitebalanceSlider_1->setEnabled(true);
        ui->exposureAutoButton_1->setEnabled(true);
        ui->whitebalanceAutoButton_1->setEnabled(true);
        for(int i=0;i<3;i++)
            syncModeButtons[1]->button(i)->setEnabled(false);
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
        ui->exposureSpinBox_2->setEnabled(false);
        ui->whitebalanceSlider_2->setEnabled(false);
        ui->exposureAutoButton_2->setEnabled(false);
        ui->whitebalanceAutoButton_2->setEnabled(false);
        for(int i=0;i<3;i++)
            syncModeButtons[2]->button(i)->setEnabled(true);
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
        k4aDevices[2]->setExposureTime(ui->exposureSpinBox_2->value());
        k4aDevices[2]->setWhiteBalance(ui->whitebalanceSlider_2->value());
        /* ui settings */
        ui->devOpenButton_2->setText("Close Device");
        ui->exposureSpinBox_2->setEnabled(true);
        ui->whitebalanceSlider_2->setEnabled(true);
        ui->exposureAutoButton_2->setEnabled(true);
        ui->whitebalanceAutoButton_2->setEnabled(true);
        for(int i=0;i<3;i++)
            syncModeButtons[2]->button(i)->setEnabled(false);
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
        ui->exposureSpinBox_3->setEnabled(false);
        ui->whitebalanceSlider_3->setEnabled(false);
        ui->exposureAutoButton_3->setEnabled(false);
        ui->whitebalanceAutoButton_3->setEnabled(false);
        for(int i=0;i<3;i++)
            syncModeButtons[3]->button(i)->setEnabled(true);

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
        k4aDevices[3]->setExposureTime(ui->exposureSpinBox_3->value());
        k4aDevices[3]->setWhiteBalance(ui->whitebalanceSlider_3->value());
        /* ui settings */
        ui->devOpenButton_3->setText("Close Device");
        ui->exposureSpinBox_3->setEnabled(true);
        ui->whitebalanceSlider_3->setEnabled(true);
        ui->exposureAutoButton_3->setEnabled(true);
        ui->whitebalanceAutoButton_3->setEnabled(true);
        for(int i=0;i<3;i++)
            syncModeButtons[3]->button(i)->setEnabled(false);
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

void MainWindow::on_exposureSpinBox_0_valueChanged(int arg1)
{
    k4aDevices[0]->setExposureTime(arg1);
}

void MainWindow::on_whitebalanceSlider_0_sliderMoved(int position)
{
    k4aDevices[0]->setWhiteBalance(position);
    ui->whitebalanceLabel_0->setText(QString::number(position));
}

void MainWindow::on_exposureSpinBox_1_valueChanged(int arg1)
{
    k4aDevices[1]->setExposureTime(arg1);
}

void MainWindow::on_whitebalanceSlider_1_sliderMoved(int position)
{
    k4aDevices[1]->setWhiteBalance(position);
    ui->whitebalanceLabel_1->setText(QString::number(position));
}

void MainWindow::on_exposureSpinBox_2_valueChanged(int arg1)
{
    k4aDevices[2]->setExposureTime(arg1);
}

void MainWindow::on_whitebalanceSlider_2_sliderMoved(int position)
{
    k4aDevices[2]->setWhiteBalance(position);
    ui->whitebalanceLabel_2->setText(QString::number(position));
}

void MainWindow::on_exposureSpinBox_3_valueChanged(int arg1)
{
    k4aDevices[3]->setExposureTime(arg1);
}

void MainWindow::on_whitebalanceSlider_3_sliderMoved(int position)
{
    k4aDevices[3]->setWhiteBalance(position);
    ui->whitebalanceLabel_3->setText(QString::number(position));
}

void MainWindow::on_exposureAutoButton_0_clicked()
{
    if(ui->exposureAutoButton_0->text()=="M")
    {
        // 自动模式下会从设定值开始自动调整
        k4aDevices[0]->setExposureTime(DEFAULT_EXPOSURE_EXP,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->exposureAutoButton_0->setText("A");
        ui->exposureSpinBox_0->setEnabled(false);
    }
    else
    {
        k4aDevices[0]->setExposureTime(ui->exposureSpinBox_0->value());
        ui->exposureAutoButton_0->setText("M");
        ui->exposureSpinBox_0->setEnabled(true);
    }
}

void MainWindow::on_whitebalanceAutoButton_0_clicked()
{
    if(ui->whitebalanceAutoButton_0->text()=="M")
    {
        // 自动模式下会从设定值开始自动调整
        k4aDevices[0]->setWhiteBalance(DEFAULT_WHITEBALANCE,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->whitebalanceAutoButton_0->setText("A");
        ui->whitebalanceSlider_0->setEnabled(false);
    }
    else
    {
        k4aDevices[0]->setWhiteBalance(ui->whitebalanceSlider_0->value());
        ui->whitebalanceAutoButton_0->setText("M");
        ui->whitebalanceSlider_0->setEnabled(true);
    }
}

void MainWindow::on_exposureAutoButton_1_clicked()
{
    if(ui->exposureAutoButton_1->text()=="M")
    {
        k4aDevices[1]->setExposureTime(DEFAULT_EXPOSURE_EXP,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->exposureAutoButton_1->setText("A");
        ui->exposureSpinBox_1->setEnabled(false);
    }
    else
    {
        k4aDevices[1]->setExposureTime(ui->exposureSpinBox_1->value());
        ui->exposureAutoButton_1->setText("M");
        ui->exposureSpinBox_1->setEnabled(true);
    }
}

void MainWindow::on_whitebalanceAutoButton_1_clicked()
{
    if(ui->whitebalanceAutoButton_1->text()=="M")
    {
        k4aDevices[1]->setWhiteBalance(DEFAULT_WHITEBALANCE,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->whitebalanceAutoButton_1->setText("A");
        ui->whitebalanceSlider_1->setEnabled(false);
    }
    else
    {
        k4aDevices[1]->setWhiteBalance(ui->whitebalanceSlider_1->value());
        ui->whitebalanceAutoButton_1->setText("M");
        ui->whitebalanceSlider_1->setEnabled(true);
    }
}

void MainWindow::on_exposureAutoButton_2_clicked()
{
    if(ui->exposureAutoButton_2->text()=="M")
    {
        k4aDevices[2]->setExposureTime(DEFAULT_EXPOSURE_EXP,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->exposureAutoButton_2->setText("A");
        ui->exposureSpinBox_2->setEnabled(false);
    }
    else
    {
        k4aDevices[2]->setExposureTime(ui->exposureSpinBox_2->value());
        ui->exposureAutoButton_2->setText("M");
        ui->exposureSpinBox_2->setEnabled(true);
    }
}

void MainWindow::on_whitebalanceAutoButton_2_clicked()
{
    if(ui->whitebalanceAutoButton_2->text()=="M")
    {
        k4aDevices[2]->setWhiteBalance(DEFAULT_WHITEBALANCE,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->whitebalanceAutoButton_2->setText("A");
        ui->whitebalanceSlider_2->setEnabled(false);
    }
    else
    {
        k4aDevices[2]->setWhiteBalance(ui->whitebalanceSlider_2->value());
        ui->whitebalanceAutoButton_2->setText("M");
        ui->whitebalanceSlider_2->setEnabled(true);
    }
}

void MainWindow::on_exposureAutoButton_3_clicked()
{
    if(ui->exposureAutoButton_3->text()=="M")
    {
        k4aDevices[3]->setExposureTime(DEFAULT_EXPOSURE_EXP,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->exposureAutoButton_3->setText("A");
        ui->exposureSpinBox_3->setEnabled(false);
    }
    else
    {
        k4aDevices[3]->setExposureTime(ui->exposureSpinBox_3->value());
        ui->exposureAutoButton_3->setText("M");
        ui->exposureSpinBox_3->setEnabled(true);
    }
}

void MainWindow::on_whitebalanceAutoButton_3_clicked()
{
    if(ui->whitebalanceAutoButton_3->text()=="M")
    {
        k4aDevices[3]->setWhiteBalance(DEFAULT_WHITEBALANCE,K4A_COLOR_CONTROL_MODE_AUTO);
        ui->whitebalanceAutoButton_3->setText("A");
        ui->whitebalanceSlider_3->setEnabled(false);
    }
    else
    {
        k4aDevices[3]->setWhiteBalance(ui->whitebalanceSlider_3->value());
        ui->whitebalanceAutoButton_3->setText("M");
        ui->whitebalanceSlider_3->setEnabled(true);
    }
}
