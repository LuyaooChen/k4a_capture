#include <k4a/k4a.hpp>

#include <iostream>
#include <vector>
#include <array>

#include <Eigen/Dense>
#include <Eigen/Core>

#include <opencv2/core.hpp>
#include <opencv2/core/eigen.hpp>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
// #include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/common/transforms.h>

#include "k4a_grabber.h"

using namespace std;
using namespace boost;
using namespace pcl;

typedef pcl::PointXYZRGBA PointType;

Eigen::Matrix4d load_transformMat(const std::string &path)
{
    cv::FileStorage freader;
    freader.open(path, cv::FileStorage::READ);
    cv::Mat transformMat_;
    freader["transformMat"] >> transformMat_;
    Eigen::Matrix4d transformMat;
    cv::cv2eigen(transformMat_, transformMat);
    return transformMat;
}

int main(int argc, char **argv)
{
    // Eigen::Matrix4d transformMat = load_transformMat("../calibration_parameters.yaml");

    const uint32_t deviceCount = k4a::device::get_installed_count();
    if (deviceCount == 0)
    {
        cout << "no azure kinect devices detected!" << endl;
    }

    // PCL Visualizer
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("Point Cloud Viewer"));

    // Point Cloud
    pcl::PointCloud<PointType>::Ptr cloud_1;
    // pcl::PointCloud<PointType>::Ptr cloud_2;

    // Retrieved Point Cloud Callback Function
    boost::mutex mutex_1;
    boost::function<void(const pcl::PointCloud<PointType>::ConstPtr &)> function_1 =
        [&cloud_1, &mutex_1](const pcl::PointCloud<PointType>::ConstPtr &ptr) {
            boost::mutex::scoped_lock lock(mutex_1);

            /* Point Cloud Processing */

            cloud_1 = ptr->makeShared();

            pcl::VoxelGrid<PointType> sampler;
            sampler.setInputCloud(cloud_1);
            sampler.setLeafSize(0.008f, 0.008f, 0.008f);
            sampler.filter(*cloud_1);
        };

    // boost::mutex mutex_2;
    // boost::function<void(const pcl::PointCloud<PointType>::ConstPtr &)> function_2 =
    //     [&cloud_2, &mutex_2](const pcl::PointCloud<PointType>::ConstPtr &ptr) {
    //         boost::mutex::scoped_lock lock(mutex_2);

    //         /* Point Cloud Processing */

    //         cloud_2 = ptr->makeShared();

    //         pcl::VoxelGrid<PointType> sampler;
    //         sampler.setInputCloud(cloud_2);
    //         sampler.setLeafSize(0.008f, 0.008f, 0.008f);
    //         sampler.filter(*cloud_2);
    //     };

    // KinectAzureDKGrabber
    boost::shared_ptr<pcl::Grabber> grabber_1 =
        boost::make_shared<pcl::KinectAzureDKGrabber>(0, K4A_DEPTH_MODE_WFOV_2X2BINNED, K4A_IMAGE_FORMAT_COLOR_BGRA32, K4A_COLOR_RESOLUTION_720P);
    // boost::shared_ptr<pcl::Grabber> grabber_2 =
    //     boost::make_shared<pcl::KinectAzureDKGrabber>(1, K4A_DEPTH_MODE_WFOV_2X2BINNED, K4A_IMAGE_FORMAT_COLOR_BGRA32, K4A_COLOR_RESOLUTION_720P);

    boost::shared_ptr<pcl::KinectAzureDKGrabber> grabber_1_ = boost::dynamic_pointer_cast<pcl::KinectAzureDKGrabber>(grabber_1);
    // boost::shared_ptr<pcl::KinectAzureDKGrabber> grabber_2_ = boost::dynamic_pointer_cast<pcl::KinectAzureDKGrabber>(grabber_2);

    // Register Callback Function
    boost::signals2::connection connection_1 = grabber_1->registerCallback(function_1);
    // boost::signals2::connection connection_2 = grabber_2->registerCallback(function_2);

    // Start Grabber
    grabber_1->start();
    // grabber_2->start();

    k4a::calibration calibration_1 = grabber_1_->getCalibration();
    // k4a::calibration calibration_2 = grabber_2_->getCalibration();
    k4a_calibration_intrinsic_parameters_t *intrinsics = &calibration_1.color_camera_calibration.intrinsics.parameters;
    Eigen::Matrix3f intrinsics_eigen_1;
    intrinsics_eigen_1 << intrinsics->param.fx, 0.0f, intrinsics->param.cx,
        0.0f, intrinsics->param.fy, intrinsics->param.cy,
        0.0f, 0.0f, 1.0f;
    // intrinsics = &calibration_2.color_camera_calibration.intrinsics.parameters;
    // Eigen::Matrix3f intrinsics_eigen_2;
    // intrinsics_eigen_2 << intrinsics->param.fx, 0.0f, intrinsics->param.cx,
    //     0.0f, intrinsics->param.fy, intrinsics->param.cy,
    //     0.0f, 0.0f, 1.0f;

    Eigen::Matrix4f extrinsics_eigen = Eigen::Matrix4f::Identity();
    viewer->setCameraParameters(intrinsics_eigen_1, extrinsics_eigen);

    pcl::PointCloud<PointType>::Ptr cloud(new PointCloud<PointType>);

    while (!viewer->wasStopped())
    {
        // Update Viewer
        viewer->spinOnce();
        boost::mutex::scoped_try_lock lock_1(mutex_1);
        // boost::mutex::scoped_try_lock lock_2(mutex_2);
        if (lock_1.owns_lock() && cloud_1 )//&& lock_2.owns_lock() && cloud_2)
        {
            //transform cloud_2 to cloud_1 coordinate
            // transformPointCloud(*cloud_2, *cloud_2, transformMat.cast<float>(), true);

            *cloud = *cloud_1 ; //+ *cloud_2;

            // Update Point Cloud
            if (!viewer->updatePointCloud(cloud, "cloud"))
            {
                viewer->addPointCloud(cloud, "cloud");
            }
        }
    }

    // Stop Grabber
    grabber_1->stop();
    // grabber_2->stop();

    // Disconnect Callback Function
    if (connection_1.connected())
    {
        connection_1.disconnect();
    }
    // if (connection_2.connected())
    // {
    //     connection_2.disconnect();
    // }
    return 0;
}
