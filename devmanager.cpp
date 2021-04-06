#include "devmanager.h"
#include <QDebug>
#include <open3d/Open3D.h>

devManager::devManager() : _is_running(false), _is_viewerOpened(false)
{
    for(int i=0;i<N_CAM;i++)
    {
        k4aDevices[i] = new k4aDevice(i);
        pointcloud[i] = std::shared_ptr<open3d::geometry::PointCloud>(new open3d::geometry::PointCloud());
    }
}

devManager::~devManager()
{
    _is_running=false;
    for(int i=0;i<N_CAM;i++)
    {
        delete k4aDevices[i];
    }
    msleep(25); // 等待线程结束
}

void devManager::begin()
{
    start();
    _is_running=true;
}

void devManager::stop()
{
    _is_running=false;
}

void devManager::colored_icp()
{
//    open3d::geometry::PointCloud p;
}

void devManager::setVisualMode(visualization_mode_t mode)
{
    if(mode==VISUALIZATION_MODE_2D)
    {
        for(int i=0;i<N_CAM;i++)
        {
            k4aDevices[i]->setVisualMode(mode);
        }
    }
    else
    {
        for(int i=0;i<N_CAM;i++)
        {
            k4aDevices[i]->setVisualMode(mode);
        }
    }
    visualization_mode=mode;
}

visualization_mode_t devManager::getVisualMode() const
{
    return visualization_mode;
}

void devManager::run()
{
    while(_is_running)
    {
        bool is_valid=false;
        for(int i=0;i<N_CAM;i++)
        {
            if(k4aDevices[i]->is_camRunning())
            {
                k4aDevices[i]->start();
                is_valid=true;
            }
        }
        if(!is_valid)
        {
            // 如果没有相机在运行，等一会儿
            msleep(20);
            continue;
        }
        while(1)
        {
            bool is_allFinished=true;
            for(int i=0;i<N_CAM;i++)
            {
                if(k4aDevices[i]->isRunning())
                {
                    is_allFinished=false;
                }
            }
            // 如果所有相机都完成了,break，开始下一次采集
            if(is_allFinished)
            {
                /** 对所有图像共同的处理写在这 **/
                if(visualization_mode==VISUALIZATION_MODE_3D)
                {
                    mutex.lock();
                    for(int i=0;i<N_CAM;i++)
                        *(pointcloud[i]) = *(k4aDevices[i]->getPointCloud());
                    mutex.unlock();
                    emit sig_SendPointCloudReady(true);
                }

                break;
            }
            msleep(1);  // 稍微睡眠一下避免一直在无意义循环
        }
    }
}
