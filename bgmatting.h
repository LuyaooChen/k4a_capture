#ifndef BGMATTING_H
#define BGMATTING_H
#include <torch/script.h>
#include <opencv2/opencv.hpp>

class BgMatting
{
public:
    BgMatting(const std::string& dev="CUDA");
    void load(const std::string& path);
    cv::Mat forward(const cv::Mat img, const cv::Mat bgr);
    bool is_valid();
private:
    torch::jit::Module model;
    torch::Device device;
    bool is_loaded;
};

#endif // BGMATTING_H
