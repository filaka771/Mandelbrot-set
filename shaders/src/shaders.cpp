#include "ppmlib.h"
#include <cmath>
namespace shaders{
    ppmimg::color grey(int escape_time, int max_time) {
        ppmimg::color color;

        float fmax_time    = static_cast<float>(max_time);
        float fescape_time = static_cast<float>(escape_time);

        fescape_time = (fescape_time / max_time) * 255;

        color.r = static_cast<uint32_t>(fescape_time);
        color.g = static_cast<uint32_t>(fescape_time);
        color.b = static_cast<uint32_t>(fescape_time);

        return color;
    }

    ppmimg::color indigo(int escape_time, int max_time){
        ppmimg::color color;

        float fmax_time    = static_cast<float>(max_time);
        float fescape_time = static_cast<float>(escape_time);

        //float normed_escape_time = fescape_time / max_time;

        if(escape_time == max_time){
            color = {0, 0, 0};
        }
        else{
            color.r = 255 * (0.5 + 0.5 * std::cos(3.0 + fescape_time * 0.15));
            color.g = 255 * (0.5 + 0.5 * std::cos(3.0 + fescape_time * 0.15 + 0.6));
            color.b = 255 * (0.5 + 0.5 * std::cos(3.0 + fescape_time * 0.15 + 1.0));
        }

        return color;
    }
}
