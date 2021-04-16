#ifndef DEVMANAGER_H
#define DEVMANAGER_H
#include <QThread>
#include <QMutex>
#include "k4adevice.h"

#define N_CAM 4

class devManager : public QThread
{
    Q_OBJECT
public:
    devManager();
    ~devManager();

    k4aDevice* k4aDevices[N_CAM];
    void begin();
    void stop();
    void setVisualMode(visualization_mode_t mode);
    visualization_mode_t getVisualMode() const;
    void loadBGMModel(const std::string& path);

    QMutex mutex;
    bool _is_viewerOpened;
    bool refineRegistration_on;
    bool saveAllImgs_on;
    bool setBG_on;
    std::shared_ptr<open3d::geometry::PointCloud> pointcloud[N_CAM];

private:
    void run() override;
    bool _is_running;
    Eigen::Matrix4d colored_icp(std::shared_ptr<open3d::geometry::PointCloud> src, std::shared_ptr<open3d::geometry::PointCloud> tar);
    visualization_mode_t visualization_mode;
    BgMatting bgm;

Q_SIGNALS:
    void sig_SendPointCloudReady(bool);
    void sig_FPS(float);

};

#endif // DEVMANAGER_H
