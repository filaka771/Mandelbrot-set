#include "ppmlib.h"
#include "shaders.h"
#include "mandelbrot.h"

int main() {
    auto set = mandelbrot<float>(2160, 2160);

    set.set_viewport(-2.5f, 1.0f, -2.0f, 2.0f);
    set.render();
    set.save_image<shaders::grey>("../examples/mandelbrot.ppm", 255);
    return 0;
}
