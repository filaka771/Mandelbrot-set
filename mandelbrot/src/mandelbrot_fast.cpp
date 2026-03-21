#include <shaders.h>
#include "mandelbrot_fast.h"   // your header

int main() {
    const uint32_t WIDTH  = 2160;
    const uint32_t HEIGHT = 2160;

    mandelbrot_fast set(WIDTH, HEIGHT);

    set.set_viewport(-2.5f, 1.5f, -2.0f, 2.0f);
    set.render();
    set.save_image<shaders::grey>("../examples/mandelbrot_fast.ppm", 255);

    return 0;
}
