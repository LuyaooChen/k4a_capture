#include "bgmatting.h"
#include <iostream>

BgMatting::BgMatting(const std::string& dev):device(dev), is_loaded(false)
{}

void BgMatting::load(const std::string& path)
{
    model = torch::jit::load(path);
    model.setattr("backbone_scale", 0.375);
    model.setattr("refine_mode", "sampling");
    model.setattr("refine_sample_pixels", 53333);
    model.to(device);
    model.eval();

    is_loaded=true;
}

bool BgMatting::is_valid()
{
    return is_loaded;
}

// 在调用之前应该检查is_valid
cv::Mat BgMatting::forward(const cv::Mat img, const torch::Tensor &bgrTensor)
{
    if(img.rows!=bgrTensor.sizes()[2] || img.cols!=bgrTensor.sizes()[3])
        throw std::runtime_error("current image size doesn't match to bcakground image size!");

    torch::Tensor imgTensor=torch::from_blob(img.data,{1,img.rows,img.cols,3},torch::kU8);
    // std::cout << imgTensor.sizes() << std::endl;
    imgTensor = imgTensor.permute({0, 3, 1, 2}).to(device).to(torch::kFloat16).div(255.0);

    auto outputs = model.forward({imgTensor, bgrTensor}).toTuple()->elements();
    auto phaTensor = outputs[0].toTensor().squeeze().detach();
    phaTensor=phaTensor.mul(255).clamp(0,255).to(torch::kU8).to(torch::kCPU);
//    auto fgr = outputs[1].toTensor();

    cv::Mat ret(phaTensor.sizes()[0],phaTensor.sizes()[1],CV_8UC1);
    std::memcpy(ret.data,phaTensor.data_ptr(),sizeof(torch::kU8)*phaTensor.numel());
    return ret;
}
