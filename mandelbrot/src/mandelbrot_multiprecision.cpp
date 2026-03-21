#include "ppmlib.h"
#include <boost/multiprecision/cpp_bin_float.hpp>
#include "shaders.h"
#include "mandelbrot.h"

using Float = boost::multiprecision::cpp_bin_float_100;

struct render_confing {
    Float min_real;
    Float max_real;
    Float min_imag;
    Float max_imag;
    int max_iterations;
};

int main(){
    render_confing deep_zoom_1 = {
        Float("-1.29518908214777745701706417718568192670656646088488846921745350") - Float("2.55E-55"),
        Float("-1.29518908214777745701706417718568192670656646088488846921745350") + Float("2.55E-55"),
        Float("+0.44093698267832013888090367835626261211321462743139620368266100") - Float("2.55E-55"),
        Float("+0.44093698267832013888090367835626261211321462743139620368266100") + Float("2.55E-55"),
        2000
    };
    
    auto set = mandelbrot<Float>(2160, 2160);
    set.set_viewport(deep_zoom_1.min_real, deep_zoom_1.max_real, deep_zoom_1.min_imag, deep_zoom_1.max_imag);
    set.set_calculation_iterations(15000);
    set.render(true); // True make calculate parallel
    set.save_image<shaders::indigo>("../examples/multy_mandelbrot_deep_zoom_1.ppm", 255);
    return 0;

}
