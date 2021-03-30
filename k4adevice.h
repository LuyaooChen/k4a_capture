#ifndef K4ADEVICE_H
#define K4ADEVICE_H
#include <QThread>
#include <QImage>
#include <k4a/k4a.hpp>

class k4aDevice: public QThread
{
    Q_OBJECT
public:
    k4aDevice(uint32_t index);
    ~k4aDevice();
    void startCamera();
    void stopCamera();
    bool open();
    void close();
    bool is_opened();
    bool is_camRunning();
    uint32_t getDeviceId();
    void setSyncMode(k4a_wired_sync_mode_t m);

private:
    void run() override;

    uint32_t device_index;

    k4a::device device;
    k4a_device_configuration_t config;
    k4a::transformation transformation;
    k4a::capture capture;
    k4a::image depthImage;
    k4a::image colorImage;
    k4a::image transformed_depth_image;

    bool _is_opened;
    bool _is_camRunning;

    static constexpr int K4A_COLOR_RESOLUTIONS[7][2]= {{0,0}, {1280,720},{1920,1080},{2560,1440},{2048,1536},{3840,2160},{4096,3072}};

signals:
    void sig_SendColorImg(QImage);
    void sig_SendDepthImg(QImage);
};

static QVector<QRgb> colorTable;

#endif // K4ADEVICE_H
