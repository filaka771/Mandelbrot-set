#include "ppmlib.h"
#include "shaders.h"
#include "mandelbrot.h"

int main() {
    auto set = mdb<float>(3840, 2160);
    set.set_viewport(-2.5, 1, -1.25, 1.25);
    set.render();
    set.save_image<shaders::indigo>("mandelbrot.ppm", 255);
    return 0;
}
