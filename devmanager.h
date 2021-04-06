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

    QMutex mutex;
    bool _is_viewerOpened;
    std::shared_ptr<open3d::geometry::PointCloud> pointcloud[N_CAM];

private:
    void run() override;
    bool _is_running;
    void colored_icp();
    visualization_mode_t visualization_mode;

signals:
    void sig_SendPointCloudReady(bool);

};

#endif // DEVMANAGER_H
