#include "k4adevice.h"
#include <QMatrix>
#include "depthcolorizer.h"
#include <QDebug>

constexpr int k4aDevice::K4A_COLOR_RESOLUTIONS[7][2];

k4aDevice::k4aDevice(uint32_t index) : device_index(index), _is_opened(false), _is_camRunning(false)
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
        device = k4a::device::open(device_index);
    }
    catch(std::exception& e){
        return false;
    }
    _is_opened=true;
    return true;
}

bool k4aDevice::is_opened()
{
    return _is_opened;
}

bool k4aDevice::is_camRunning()
{
    return _is_camRunning;
}

void k4aDevice::close()
{
    if(_is_camRunning) stopCamera();
    device.close();
    _is_opened=false;
}

void k4aDevice::startCamera()
{
    device.start_cameras(&config);
    k4a::calibration calibration = device.get_calibration(config.depth_mode, config.color_resolution);
    transformation = k4a::transformation(calibration);
    start();
    _is_camRunning=true;
}

void k4aDevice::stopCamera()
{
    _is_camRunning=false;
    msleep(30);
    device.stop_cameras();
    transformation.destroy();

}

uint32_t k4aDevice::getDeviceId()
{
    return device_index;
}

void k4aDevice::setSyncMode(k4a_wired_sync_mode_t m)
{
    //todo: 检查类型是否正确
    config.wired_sync_mode=m;
    if(m==K4A_WIRED_SYNC_MODE_SUBORDINATE)
        config.subordinate_delay_off_master_usec=160;
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
