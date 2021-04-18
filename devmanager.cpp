#include "devmanager.h"
#include <QDebug>
#include <QTime>
#include <open3d/Open3D.h>
#include <open3d/pipelines/registration/ColoredICP.h>

devManager::devManager() : _is_running(false),
    _is_viewerOpened(false),
    refineRegistration_on(false),
    saveAllImgs_on(false),
    setBG_on(false),
    bgm("cuda")
{
    for(int i=0;i<N_CAM;i++)
    {
        k4aDevices[i] = new k4aDevice(i);
        k4aDevices[i]->applyBgMatting(&bgm);
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

Eigen::Matrix4d devManager::colored_icp(std::shared_ptr<open3d::geometry::PointCloud> src,
                             std::shared_ptr<open3d::geometry::PointCloud> tar)
{
    open3d::geometry::PointCloud src_t= *src;
    open3d::geometry::PointCloud tar_t= *tar;
    open3d::pipelines::registration::RegistrationResult result;
    std::vector<float> voxel_radius{0.04,0.02,0.01};
    std::vector<int> max_iter{50,30,14};
    for(int i=0;i<max_iter.size();++i)
    {
        float radius=voxel_radius[i];
        int iter=max_iter[i];
        qDebug()<<"downsampling...";
        std::shared_ptr<open3d::geometry::PointCloud> src_down=src_t.VoxelDownSample(radius);
        std::shared_ptr<open3d::geometry::PointCloud> tar_down=tar_t.VoxelDownSample(radius);
        qDebug()<<"estimating normals...";
        src_down->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(radius*2,30));
        tar_down->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(radius*2,30));
        qDebug()<<"colored ICP ...";
        result=open3d::pipelines::registration::RegistrationColoredICP(*src_down,*tar_down,
                                                                       radius,Eigen::Matrix4d::Identity(),
                                                                       open3d::pipelines::registration::TransformationEstimationForColoredICP(),
                                                                       open3d::pipelines::registration::ICPConvergenceCriteria(1e-6,1e-6,iter));
    }
    return result.transformation_;
}

void devManager::setVisualMode(visualization_mode_t mode)
{
    for(int i=0;i<N_CAM;i++)
    {
        k4aDevices[i]->setVisualMode(mode);
    }
    visualization_mode=mode;
}

visualization_mode_t devManager::getVisualMode() const
{
    return visualization_mode;
}

void devManager::loadBGMModel(const std::string &path)
{
    bgm.load(path);
}

void devManager::run()
{
    QTime time;
    time.start();
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
                    break;
                }
            }
            // 如果所有相机都完成了,break，开始下一次采集
            if(is_allFinished)
            {
                /** 对所有图像共同的处理写在这,以避免线程同步问题 **/
                if(visualization_mode==VISUALIZATION_MODE_3D)
                {
                    mutex.lock();
                    for(int i=0;i<N_CAM;i++)
                        if(k4aDevices[i]->is_camRunning())
                        {
                            *(pointcloud[i]) = *(k4aDevices[i]->getPointCloud());
                        }
                    // Colored_ICP 优化配准
                    if(refineRegistration_on)
                    {
                        refineRegistration_on=false;
                        Eigen::Matrix4d transformation_current;
                        Eigen::Matrix4d transformation_result;
                        std::shared_ptr<open3d::geometry::PointCloud> pc_sum(new open3d::geometry::PointCloud());
                        for(int i=0;i<N_CAM;i++)
                        {
                            if(k4aDevices[i]->is_camRunning())
                            {
                                qDebug()<<"正在处理点云: "<<QString::number(i);
                                if(pc_sum->IsEmpty())
                                {
                                    *pc_sum=*(pointcloud[i]);
                                    transformation_current=k4aDevices[i]->getColorExtrinsicMatrix();
                                    continue;
                                }
                                transformation_result=colored_icp(pointcloud[i], pc_sum);
                                pointcloud[i]->Transform(transformation_result);
                                k4aDevices[i]->setColorExtrinsicMatrix(transformation_result*transformation_current);
                                *pc_sum += *(pointcloud[i]);
                            }
                        }
                    }

                    mutex.unlock();
                    Q_EMIT sig_SendPointCloudReady(true);
                }

                if(saveAllImgs_on)
                {
                    saveAllImgs_on=false;
                    for(int i=0;i<N_CAM;i++)
                        k4aDevices[i]->saveImg();
                }

                if(setBG_on)
                {
                    setBG_on=false;
                    for(int i=0;i<N_CAM;i++)
                        if(k4aDevices[i]->is_camRunning())
                            k4aDevices[i]->setBackground(k4aDevices[i]->getColorImg());
                }

                break;
            }
            msleep(1);  // 稍微睡眠一下避免一直在无意义循环
        }   // while(1)
        Q_EMIT sig_FPS(1000/(float)time.restart());
    }   // while(_is_running)
}
