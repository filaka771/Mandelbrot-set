#include <cstdio>
#include "mandelbrot_perturb.h"
#include "shaders.h"  // Your shader functions

int main() {
    // Use double precision for best GPU compatibility
    mdb<double> render(3840, 2160);  // 4K resolution
    
    // Set view: classic Mandelbrot region
    render.set_viewport(-2.5, 1.0, -1.75, 1.75);
    
    // Render with GPU perturbation
    printf("Rendering with GPU...\n");
    render.renderGPU(1000);  // 1000 max iterations
    
    // Save image
    printf("Saving image...\n");
    render.save_image<shaders::grey>("mandelbrot_gpu.ppm", 255);
    
    printf("Done!\n");
    return 0;
}
