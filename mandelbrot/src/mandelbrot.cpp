#include "ppmlib.h"
#include "shaders.h"
#include "mandelbrot.h"
#include <vector>

#include <algorithm>
#include <iostream>

int main() {
    auto set = mdb<float>(3840, 2160);
    set.set_viewport(-2.5, 1, -1.25, 1.25);
    set.calculate();

    const auto& escape_times = set.get_escape_time();

    auto min = min_element(escape_times.begin(), escape_times.end());
    auto max = max_element(escape_times.begin(), escape_times.end());

    std::cout << "Min: " << *min << "Max: " << *max;

    ppmimg image(3840, 2160, 255);

    for(uint32_t y = 0; y < 2160; y ++){
        for(uint32_t x = 0; x < 3840; x ++){

            ppmimg::color pixel_color = grey(escape_times[y * 3840 + x], 128);
            image.set_pixel(ppmimg::pixel(x, y), pixel_color);
        }
    }

    image.save("mandelbfrot.ppm", false);

    return 0;
}
