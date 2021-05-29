#include "k4adevice.h"
#include <QMatrix>
#include <QDir>
#include <QTime>
//#include <QDateTime>
#include <QDebug>
#include <opencv2/core/eigen.hpp>
#include <open3d/Open3D.h>

constexpr int k4aDevice::K4A_COLOR_RESOLUTIONS[7][2];

k4aDevice::k4aDevice(uint32_t index) :
    deviceIndex(index),
    _is_opened(false),
    _is_camRunning(false),
    _is_visual(true),
    enableBgMatting(true),
    enableDepth(true),
    visualization_mode(VISUALIZATION_MODE_2D)
{
    // 如果需要gui来配置config，以下需要重写以另外配置
    config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;
    if(enableDepth) config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    else config.depth_mode = K4A_DEPTH_MODE_OFF;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_720P;
//    config.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;
//    config.subordinate_delay_off_master_usec = 160;
    if(enableDepth) config.synchronized_images_only = true;

    rotateMat.rotate(90);
    colorExtrinsicMatrix=Eigen::Matrix4d::Identity();
    o3d_pc = std::shared_ptr<open3d::geometry::PointCloud>(new open3d::geometry::PointCloud()); //切换3d模式时可能没同步上，提前初始化避免读到空指针
}

k4aDevice::~k4aDevice()
{
    close();
}

bool k4aDevice::open()
{
    try{
        device = k4a::device::open(deviceIndex);
        serialNum=device.get_serialnum();
    }
    catch(std::exception& e){
        return false;
    }
    _is_opened=true;
    return true;
}

void k4aDevice::close()
{
    if(_is_camRunning) stopCamera();
    device.close();
    _is_opened=false;
}

bool k4aDevice::is_opened() const
{
    return _is_opened;
}

bool k4aDevice::is_camRunning() const
{
    return _is_camRunning;
}

void k4aDevice::startCamera()
{
    if(!_is_camRunning)
    {
        device.start_cameras(&config);
        loadColorExtrinsicMatrix("/home/cly/workspace/k4a_capture/doc/calib/");
        device.set_color_control(K4A_COLOR_CONTROL_POWERLINE_FREQUENCY,K4A_COLOR_CONTROL_MODE_MANUAL,1);    // 电源50Hz

        k4a::calibration calibration = device.get_calibration(config.depth_mode, config.color_resolution);
        transformation = k4a::transformation(calibration);
        k4a_calibration_intrinsic_parameters_t& intri = calibration.color_camera_calibration.intrinsics.parameters;
        colorIntrinsicMatrix<<intri.param.fx, 0.0f, intri.param.cx,
                0.0f, intri.param.fy, intri.param.cy,
                0.0f, 0.0f, 1.0f;
        std::cout<<deviceIndex<<std::endl;

        std::cout<<intri.param.fx<<" "<<intri.param.cx<<" "<<intri.param.fy<<" "<<intri.param.cy<<std::endl;
        std::cout<<intri.param.k1<<" "<<intri.param.k2<<" "<<intri.param.p1<<" "<<intri.param.p2<<" "<<intri.param.k3<<" "<<intri.param.k4<<" "<<intri.param.k5<<" "<<intri.param.k6<<std::endl;

        width = K4A_COLOR_RESOLUTIONS[config.color_resolution][0];
        height = K4A_COLOR_RESOLUTIONS[config.color_resolution][1];
        transformed_depth_image = k4a::image::create(   K4A_IMAGE_FORMAT_DEPTH16,
                                                        width,
                                                        height,
                                                        width * (int)sizeof(uint16_t));

        o3d_color = std::shared_ptr<open3d::geometry::Image>(new open3d::geometry::Image());
        o3d_color->width_=width;
        o3d_color->height_=height;
        o3d_color->num_of_channels_=3;
        o3d_color->bytes_per_channel_=1; // 8bit图为1， 16bit图为2. 不是指整张图的大小

        o3d_depth = std::shared_ptr<open3d::geometry::Image>(new open3d::geometry::Image());
        o3d_depth->width_=width;
        o3d_depth->height_=height;
        o3d_depth->num_of_channels_=1;
        o3d_depth->bytes_per_channel_=2;

        if(enableDepth) colorizer = new depthColorizer(config.depth_mode);

        _is_camRunning=true;
    }
}

void k4aDevice::stopCamera()
{
    _is_camRunning=false;
//    msleep(35); // 保证capture结束了
    device.stop_cameras();
    transformation.destroy();
    if(enableDepth) delete colorizer;
}

uint32_t k4aDevice::getDeviceId() const
{
    return deviceIndex;
}
std::string k4aDevice::getDeviceSerialNum() const{
    return serialNum;
}
const Eigen::Matrix3f &k4aDevice::getColorIntrinsicMatrix() const
{
    return colorIntrinsicMatrix;
}

void k4aDevice::setExposureTime(int exp, k4a_color_control_mode_t mode)
{
    // 曝光值分档位，有如下值：
    // 详见https://docs.microsoft.com/zh-cn/azure/kinect-dk/hardware-specification#rgb-camera-exposure-time-values
    int value;
    if(mode==K4A_COLOR_CONTROL_MODE_MANUAL)
        switch(exp)
        {
        case -11:   value=500;break;
        case -10:   value=1250;break;
        case -9:    value=2500;break;
        case -8:    value=10000;break;
        case -7:    value=20000;break;
        case -6:    value=30000;break;
        case -5:    value=40000;break;
        case -4:    value=50000;break;
        case -3:    value=60000;break;
        case -2:    value=80000;break;
        case -1:    value=100000;break;
        case 0:     value=120000;break;
        case 1:     value=130000;break;
        default:
            throw std::logic_error("Invalid exposure!");
        }
    device.set_color_control(K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE,mode,value);
}

void k4aDevice::setWhiteBalance(int32_t value, k4a_color_control_mode_t mode)
{
    // value超出[2566,12500]这个范围会抛出异常
    value = std::min(value,12500);
    value = std::max(value,2566);
    device.set_color_control(K4A_COLOR_CONTROL_WHITEBALANCE,mode,value);
}

void k4aDevice::setSyncMode(k4a_wired_sync_mode_t m)
{
    //todo: 检查类型是否正确
    config.wired_sync_mode=m;
    if(m==K4A_WIRED_SYNC_MODE_SUBORDINATE)
        // 设置depth_delay_off_color_usec怎么样？
        config.subordinate_delay_off_master_usec = 160*deviceIndex;
    else config.subordinate_delay_off_master_usec = 0;  // 否则会抛出异常
}

k4a_wired_sync_mode_t k4aDevice::getSyncMode()
{
    return config.wired_sync_mode;
}

void k4aDevice::loadColorExtrinsicMatrix(QString path)
{
    path = path + QString(serialNum.c_str()) + ".yaml";
    cv::FileStorage freader;
    if(freader.open(path.toStdString(), cv::FileStorage::READ))
    {
        cv::Mat transformMat;
        freader["transformMat"] >> transformMat;
        cv::cv2eigen(transformMat,colorExtrinsicMatrix);
    }
    else qDebug()<<"load extrinsic failed! "<<path;
    freader.release();
}

void k4aDevice::setColorExtrinsicMatrix(Eigen::Matrix4d mat)
{
    colorExtrinsicMatrix=mat;
}

const Eigen::Matrix4d &k4aDevice::getColorExtrinsicMatrix() const
{
    return colorExtrinsicMatrix;
}

void k4aDevice::enableVisualization(bool flag)
{
    _is_visual=flag;
}

std::shared_ptr<open3d::geometry::PointCloud> k4aDevice::getPointCloud() const
{
    return o3d_pc;
}

void k4aDevice::setVisualMode(visualization_mode_t mode)
{
    visualization_mode=mode;
}

void k4aDevice::saveImg(QString time)
{
    if(!colorImage.is_valid()) return;
    QString path =QString("imgs/")+serialNum.c_str()+"/";
    QDir qdir;    //当前目录
    if(!qdir.exists(path))
    {
        qdir.mkpath(path);
        qDebug()<<"path doesn't exist. mkpath:"+path;
    }
//    QString time=QDateTime::currentDateTime().toString("yyyy_MM_dd-hh:mm:ss");
    uchar* color_image_data = colorImage.get_buffer();
    cv::Mat tmp(height,width,CV_8UC4,color_image_data);
    cv::cvtColor(tmp,tmp,cv::COLOR_BGRA2BGR);
    cv::imwrite((path+time+".png").toStdString(),tmp);    //crashed! maybe because to libjpeg
//    QImage QColor_image(color_image_data,width,height,QImage::Format_RGBA8888);
//    QColor_image=QColor_image.rgbSwapped();
//    QColor_image.save(path+time+".png","PNG",9);
    qDebug()<<"save color img to "+qdir.absolutePath()+"/"+path+time+".png";
}

cv::Mat k4aDevice::getColorImg()
{
    if(!colorImage.is_valid()) return cv::Mat();
    cv::Mat ret(height,width,CV_8UC4,colorImage.get_buffer());
    cv::cvtColor(ret,ret,cv::COLOR_BGRA2BGR);
    return ret;
}

// 将BgMatting对象和设备相关联
void k4aDevice::applyBgMatting(BgMatting* bgm)
{
    this->bgm = bgm;
}

void k4aDevice::setBackground(cv::Mat bgImg, torch::Device dev)
{
    backgroundImg=torch::from_blob(bgImg.data,{1,bgImg.rows,bgImg.cols,3},torch::kU8);
    backgroundImg=backgroundImg.permute({0,3,1,2}).to(dev).to(torch::kFloat16).div(255.0);
}

void k4aDevice::run()
{
    if(_is_camRunning)
    {
        try
        {
            device.get_capture(&capture);
        }
        catch(k4a::error &e)
        {
            qDebug()<<"dev"+QString::number(deviceIndex)+" fail to get capture! (Ignore it when dev closing.)";
            return;
        }

        if(enableDepth)
        {
            depthImage = capture.get_depth_image();
            transformation.depth_image_to_color_camera(depthImage, &transformed_depth_image);
        }
        colorImage = capture.get_color_image();

        QTime data_time;
        data_time.start();

        uchar* color_image_data = colorImage.get_buffer();
        // 前景分割 目前单相机15ms,4相机同时要30ms左右
        if(enableBgMatting && bgm->is_valid() && backgroundImg.sizes()[0]!=0)
        {
            cv::Mat color_tmp(height,width,CV_8UC4,color_image_data);
            cv::cvtColor(color_tmp, color_tmp, cv::COLOR_BGRA2BGR);
            bgMask = bgm->forward(color_tmp, backgroundImg);
        }

        if(_is_visual)
        {
            /*为显示做处理*/
            uchar* depth_image_data;
            if(enableDepth) depth_image_data = transformed_depth_image.get_buffer();
            if(visualization_mode==VISUALIZATION_MODE_2D)
            {
                QImage QColor_image(color_image_data,width,height,QImage::Format_RGBA8888);
                QColor_image=QColor_image.rgbSwapped().transformed(rotateMat);
                Q_EMIT sig_SendColorImg(QColor_image);

                // 这里需要rgba8888，alpha没用，只是QRgb占4个字节，对齐处理起来方便。
                if(enableDepth)
                {
                    QImage QDepth_image(width,height,QImage::Format_RGBA8888);
                    colorizer->colorize(depth_image_data,QDepth_image);
                    QDepth_image = QDepth_image.transformed(rotateMat);
                    Q_EMIT sig_SendDepthImg(QDepth_image);
                }

                if(enableBgMatting && bgm->is_valid() && backgroundImg.sizes()[0]!=0)
                {
                    QImage QMask_image(bgMask.data,bgMask.cols,bgMask.rows,QImage::Format_Grayscale8);
                    Q_EMIT sig_SendMaskImg(QMask_image.transformed(rotateMat));
                }
            }
            else    // 3D
            {
                cv::Mat tmp(height,width,CV_8UC4,color_image_data);
                cv::cvtColor(tmp,tmp,cv::COLOR_BGRA2RGB);
                o3d_color->data_ = (std::vector<uint8_t>)(tmp.reshape(1,1));
                // 注意这里用了原始数据
                o3d_depth->data_ = std::vector<uint8_t>(reinterpret_cast<uint8_t *>(depth_image_data),
                                                       reinterpret_cast<uint8_t *>(depth_image_data)+width*height*sizeof(uint16_t));
                std::shared_ptr<open3d::geometry::RGBDImage> o3d_rgbd =
                        open3d::geometry::RGBDImage::CreateFromColorAndDepth(*o3d_color,
                                                                              *o3d_depth,
                                                                              1000,
                                                                              2.5,
                                                                              false);
                open3d::camera::PinholeCameraIntrinsic o3d_Intrinsic(width,
                                                                     height,
                                                                     colorIntrinsicMatrix(0,0),
                                                                     colorIntrinsicMatrix(1,1),
                                                                     colorIntrinsicMatrix(0,2),
                                                                     colorIntrinsicMatrix(1,2));
                o3d_pc = open3d::geometry::PointCloud::CreateFromRGBDImage(*o3d_rgbd,o3d_Intrinsic);
                o3d_pc->Transform(colorExtrinsicMatrix);
//                open3d::io::WritePointCloud("test.ply",*o3d_pc);
//                qDebug()<<"3d mode running...";
            }
        }   // if(is_visual)
//        qDebug()<<"设备"+QString::number(deviceIndex)+"数据处理时间"<<data_time.elapsed()<<"ms";
    }
}
