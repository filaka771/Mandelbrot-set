#include <iostream>
#include "ppmlib.h"

int main() {
    try {
        const uint32_t width = 256;
        const uint32_t height = 256;
        const uint32_t depth = 255;

        ppmimg image(width, height, depth);

        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t r = static_cast<uint8_t>((x * 255) / (width - 1));
                uint8_t g = static_cast<uint8_t>((y * 255) / (height - 1));
                uint8_t b = 128;

                ppmimg::color pixel_color(r, g, b);

                image.set_pixel(ppmimg::pixel(x, y), pixel_color);
            }
        }

        // Save in ASCII PPM (P3)
        image.save("gradient_ascii.ppm", false);

        // Save in binary PPM (P6)
        image.save("gradient_binary.ppm", true);

        std::cout << "Gradient images saved successfully.\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
