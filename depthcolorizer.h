#ifndef DEPTHCOLORIZER_H
#define DEPTHCOLORIZER_H
#include <QVector>
#include <QRgb>
#include <QImage>
#include <k4a/k4a.hpp>

class depthColorizer
{
public:
    depthColorizer(k4a_depth_mode_t depthMode);
    void colorize(const uchar *depth_image_data, QImage& target);

private:
    uint16_t d_min;
    uint16_t d_max;

    QRgb intensity2RGB(uint16_t intensity);
    inline void ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b);
    inline std::pair<uint16_t, uint16_t> GetDepthModeRange(const k4a_depth_mode_t depthMode);
};

#endif // DEPTHCOLORIZER_H
