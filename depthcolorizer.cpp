#include "depthcolorizer.h"
#define N_depthBits 16

depthColorizer::depthColorizer(k4a_depth_mode_t depthMode)
{
    auto depthRange = GetDepthModeRange(depthMode);
    d_min=depthRange.first;
    d_max=depthRange.second;
}

void depthColorizer::colorize(const uchar * depth_image_data, QImage &target)
{
    if(target.format()!=QImage::Format_RGBA8888)
        throw std::logic_error("Format of the QImage to be colorized should be rgba8888.");
    else{
        QRgb* tar_p = reinterpret_cast<QRgb*>(target.bits());
        const uint16_t* src_p = reinterpret_cast<const uint16_t*>(depth_image_data);
        for(int i=0;i<target.width()*target.height();i++)
        {
            *tar_p = intensity2RGB(*src_p);
            tar_p++;
            src_p++;
        }
    }
}

inline std::pair<uint16_t, uint16_t> depthColorizer::GetDepthModeRange(const k4a_depth_mode_t depthMode)
{
   switch (depthMode)
   {
   case K4A_DEPTH_MODE_NFOV_2X2BINNED:
       return {(uint16_t)500, (uint16_t)5800};
   case K4A_DEPTH_MODE_NFOV_UNBINNED:
       return {(uint16_t)500, (uint16_t)4000};
   case K4A_DEPTH_MODE_WFOV_2X2BINNED:
       return {(uint16_t)250, (uint16_t)3000};
   case K4A_DEPTH_MODE_WFOV_UNBINNED:
       return {(uint16_t)250, (uint16_t)2500};

   case K4A_DEPTH_MODE_PASSIVE_IR:
   default:
       throw std::logic_error("Invalid depth mode!");
   }
}

QRgb depthColorizer::intensity2RGB(uint16_t intensity)
{
    QRgb p=qRgb(0,0,0);
    if(intensity==0) return p;

    intensity = std::min(d_max,intensity);
    intensity = std::max(d_min,intensity);

    float hue = (intensity - d_min) / static_cast<float>(d_max-d_min);

    // The 'hue' coordinate in HSV is a polar coordinate, so it 'wraps'.
    // Purple starts after blue and is close enough to red to be a bit unclear,
    // so we want to go from blue to red.  Purple starts around .6666667,
    // so we want to normalize to [0, .6666667].
    constexpr float range = 2.f / 3.f;
    hue *= range;
    // We want blue to be close and red to be far, so we need to reflect the
    // hue across the middle of the range.
//    hue = range - hue;
    float fRed = 0.f;
    float fGreen = 0.f;
    float fBlue = 0.f;
    ColorConvertHSVtoRGB(hue, 1.f, 1.f, fRed, fGreen, fBlue);

    p=qRgb(static_cast<uint8_t>(fRed*255),
           static_cast<uint8_t>(fGreen*255),
           static_cast<uint8_t>(fBlue*255));
    return p;
}

inline void depthColorizer::ColorConvertHSVtoRGB(float h, float s, float v, float &out_r, float &out_g, float &out_b)
{
    if (s == 0.0f)
    {
        // gray
        out_r = out_g = out_b = v;
        return;
    }

//    h = fmodf(h, 1.0f) / (60.0f / 360.0f);
    h *= 6;
    int i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i)
    {
    case 0:
        out_r = v;
        out_g = t;
        out_b = p;
        break;
    case 1:
        out_r = q;
        out_g = v;
        out_b = p;
        break;
    case 2:
        out_r = p;
        out_g = v;
        out_b = t;
        break;
    case 3:
        out_r = p;
        out_g = q;
        out_b = v;
        break;
    case 4:
        out_r = t;
        out_g = p;
        out_b = v;
        break;
    case 5:
    default:
        out_r = v;
        out_g = p;
        out_b = q;
        break;
    }
}
