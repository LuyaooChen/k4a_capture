#include "k4adevice.h"
#include <QMatrix>
#include "depthcolorizer.h"
#include <QDebug>
//#include <iostream>

constexpr int k4aDevice::K4A_COLOR_RESOLUTIONS[7][2];

k4aDevice::k4aDevice(uint32_t index) : deviceIndex(index), _is_opened(false), _is_camRunning(false), _is_camPause(false)
{
    // 如果需要gui来配置config，以下需要重写以另外配置
    config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_720P;
//    config.wired_sync_mode = K4A_WIRED_SYNC_MODE_STANDALONE;
//    config.subordinate_delay_off_master_usec = 160;
    config.synchronized_images_only = true;
}

k4aDevice::~k4aDevice()
{
    close();
}

bool k4aDevice::open()
{
    try{
        device = k4a::device::open(deviceIndex);
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
    device.start_cameras(&config);

    k4a::calibration calibration = device.get_calibration(config.depth_mode, config.color_resolution);
    transformation = k4a::transformation(calibration);
    k4a_calibration_intrinsic_parameters_t& intri = calibration.color_camera_calibration.intrinsics.parameters;
    colorIntrinsics<<intri.param.fx, 0.0f, intri.param.cx,
            0.0f, intri.param.fy, intri.param.cy,
            0.0f, 0.0f, 1.0f;

    device.set_color_control(K4A_COLOR_CONTROL_POWERLINE_FREQUENCY,K4A_COLOR_CONTROL_MODE_MANUAL,1);
    start();
    _is_camRunning=true;
}

void k4aDevice::stopCamera()
{
    _is_camRunning=false;
    msleep(35);
    device.stop_cameras();
    transformation.destroy();
}

uint32_t k4aDevice::getDeviceId() const
{
    return deviceIndex;
}

const Eigen::Matrix3f &k4aDevice::getColorIntrinsics() const
{
    return colorIntrinsics;
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
}

void k4aDevice::run()
{
    const int width = K4A_COLOR_RESOLUTIONS[config.color_resolution][0];
    const int height = K4A_COLOR_RESOLUTIONS[config.color_resolution][1];
    transformed_depth_image = k4a::image::create(   K4A_IMAGE_FORMAT_DEPTH16,
                                                    width,
                                                    height,
                                                    width * (int)sizeof(uint16_t));
    QMatrix rotateMat;
    rotateMat.rotate(90);
    depthColorizer colorizer(config.depth_mode);
    while(_is_camRunning)
    {
        // SUB模式的设备会因为MASTER的停止而停止？还是说即使master不进行get_capture也实际在运行？
        if(_is_camPause&&(config.wired_sync_mode==K4A_WIRED_SYNC_MODE_MASTER||config.wired_sync_mode==K4A_WIRED_SYNC_MODE_STANDALONE))
            msleep(35);
        else
        {
            if(device.get_capture(&capture))
            {
                depthImage = capture.get_depth_image();
                colorImage = capture.get_color_image();
                transformation.depth_image_to_color_camera(depthImage, &transformed_depth_image);
                const uchar * color_image_data = colorImage.get_buffer();
                const uchar * depth_image_data = transformed_depth_image.get_buffer();

                QImage QColor_image(color_image_data,width,height,QImage::Format_RGBA8888);
                QColor_image=QColor_image.rgbSwapped().transformed(rotateMat);
                emit sig_SendColorImg(QColor_image);

                // 这里需要rgba8888，alpha没用，只是QRgb占4个字节，对齐处理起来方便。
                QImage QDepth_image(width,height,QImage::Format_RGBA8888);
                colorizer.colorize(depth_image_data,QDepth_image);
                QDepth_image = QDepth_image.transformed(rotateMat);
                emit sig_SendDepthImg(QDepth_image);
            }
        }

    }

}
