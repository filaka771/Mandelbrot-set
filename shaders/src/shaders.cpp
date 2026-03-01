#include "ppmlib.h"
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
