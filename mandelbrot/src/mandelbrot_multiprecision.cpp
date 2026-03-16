#include "ppmlib.h"
#include "boost/multiprecision/mpfr.hpp"
#include "shaders.h"
#include "mandelbrot.h"

using Float = boost::multiprecision::number<boost::multiprecision::mpfr_float_backend<100>>;

int main(){
    auto set = mandelbrot<Float>(3840, 2160);
    set.set_viewport(-2.5, 1, -1.25, 1.25);
    set.render();
    set.save_image<shaders::indigo>("multy_mandelbrot.ppm", 255);
    return 0;

}
