#include <shaders.h>
#include "mandelbrot_fast.h"   // your header

int main() {
    const uint32_t WIDTH  = 3440;
    const uint32_t HEIGHT = 2080;

    mandelbrot_fast mb(WIDTH, HEIGHT);

    // Classic Mandelbrot view: centre at (-0.5, 0), width ~3
    mb.set_viewport(-2.5f, 1.5f, -1.5f, 1.5f);
    mb.render();
    mb.save_image<shaders::grey>("mandelbrot_fast.ppm", 255);

    return 0;
}
