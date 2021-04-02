#ifndef K4ADEVICE_H
#define K4ADEVICE_H
#include <QThread>
#include <QImage>
#include <k4a/k4a.hpp>
#include <eigen3/Eigen/Core>
#include "depthcolorizer.h"

class k4aDevice : public QThread
{
    Q_OBJECT
public:
    k4aDevice(uint32_t index);
    ~k4aDevice();
    void startCamera();
    void stopCamera();
    bool open();
    void close();
    bool is_opened() const;
    bool is_camRunning() const;
    uint32_t getDeviceId() const;
    const Eigen::Matrix3f &getColorIntrinsics() const;
    void setSyncMode(k4a_wired_sync_mode_t m);
    void setExposureTime(int exp, k4a_color_control_mode_t mode=K4A_COLOR_CONTROL_MODE_MANUAL);
    void setWhiteBalance(int32_t value,k4a_color_control_mode_t mode=K4A_COLOR_CONTROL_MODE_MANUAL);

private:
    void run() override;

    uint32_t deviceIndex;
    k4a::device device;
    k4a_device_configuration_t config;
    k4a::transformation transformation;
    k4a::capture capture;
    k4a::image depthImage;
    k4a::image colorImage;
    k4a::image transformed_depth_image;
    Eigen::Matrix3f colorIntrinsics;
    Eigen::Matrix4f colorExtrinsics;
    uint16_t width;
    uint16_t height;

    QMatrix rotateMat;
    depthColorizer *colorizer;

    bool _is_opened;
    bool _is_camRunning;

    static constexpr int K4A_COLOR_RESOLUTIONS[7][2]= {{0,0}, {1280,720},{1920,1080},{2560,1440},{2048,1536},{3840,2160},{4096,3072}};

signals:
    void sig_SendColorImg(QImage);
    void sig_SendDepthImg(QImage);
};

#endif // K4ADEVICE_H
