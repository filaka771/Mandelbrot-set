#pragma once
#include <vector>
#include <cstdint>
#include <variant>
#include <stdexcept>
#include <fstream>

class ppmimg{
 private:
    const uint32_t width_, height_, col_depth_;

    struct color8{
        uint8_t r;
        uint8_t g;
        uint8_t b;

        color8(uint8_t red, uint8_t green, uint8_t blue)
        :r(red), g(green), b(blue){}
    };

    struct color16{
        uint16_t r;
        uint16_t g;
        uint16_t b;

        color16(uint16_t red, uint16_t green, uint16_t blue)
        :r(red), g(green), b(blue){}
    };

    std::vector<std::variant<color8, color16>> pixel_list_;

public:
    struct pixel {
        uint32_t x, y;

        pixel() = default;
        pixel(uint32_t x_coord, uint32_t y_coord)
            : x(x_coord), y(y_coord){}
    };

    struct color {
        uint32_t r, g, b;

        color() = default;
        color(uint32_t red, uint32_t green, uint32_t blue)
            : r(red), g(green), b(blue){}
    };

    ppmimg(uint32_t w, uint32_t h, uint32_t cd):
        width_(w), height_(h), col_depth_(cd){
        if(col_depth_ <= 255){
            pixel_list_.resize(width_ * height_, color8(0,0,0));
        }

        else if(col_depth_ <= 255 * 255){
            pixel_list_.resize(width_ * height_, color16(0, 0, 0));
        }

        else{
            // TODO: Find out, how constructor would behave if it cannot 
            // initialize class with provided parameters.
            throw std::invalid_argument("Color depth could not be bigger then 16 bit!");
        }
    }

    uint32_t width() const {
        return width_;
    }

    uint32_t height() const {
        return height_;
    }

    uint32_t col_depth() const{
        return col_depth_;
    }

    void set_pixel(pixel p, const color& col) {
        if (p.x >= width_ || p.y >= height_)
            throw std::out_of_range("Pixel coordinates out of bounds");

        // Validate that each component fits the image's color depth
        if (col.r > col_depth_ || col.g > col_depth_ || col.b > col_depth_)
            throw std::invalid_argument("Color value exceeds image depth");

        size_t idx = p.y * width_ + p.x;

        if (col_depth_ <= 255) {
            // Store as 8 bit
            pixel_list_[idx] = color8(
                                      static_cast<uint8_t>(col.r),
                                      static_cast<uint8_t>(col.g),
                                      static_cast<uint8_t>(col.b)
                                      );
        } else {
            // Store as 16 bit
            pixel_list_[idx] = color16(
                                       static_cast<uint16_t>(col.r),
                                       static_cast<uint16_t>(col.g),
                                       static_cast<uint16_t>(col.b)
                                       );
        }
    }

    void save(const char* filename, bool bin) {
        if (bin)
            save_p6(filename);
        else
            save_p3(filename);
    }

private:
    void save_p3(const char* filename) {
        std::ofstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Cannot open file");

        file << "P3\n" << width_ << ' ' << height_ << "\n" << col_depth_ << "\n";

        for (uint32_t y = 0; y < height_; ++y) {
            for (uint32_t x = 0; x < width_; ++x) {
                const auto& pixel = pixel_list_[y * width_ + x];
                std::visit([&file](const auto& c) {
                    file << static_cast<int>(c.r) << ' '
                    << static_cast<int>(c.g) << ' '
                    << static_cast<int>(c.b);
                }, pixel);
                file << '\n';
            }
        }
    }

    void save_p6(const char* filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file)
            throw std::runtime_error("Cannot open file for writing");

        file << "P6\n" << width_ << ' ' << height_ << '\n' << col_depth_ << '\n';

        for (uint32_t y = 0; y < height_; ++y) {
            for (uint32_t x = 0; x < width_; ++x) {
                const auto& pixel = pixel_list_[y * width_ + x];
                std::visit([&file](const auto& c) {
                    using T = std::decay_t<decltype(c)>;
                    if constexpr (std::is_same_v<T, color8>) {
                        uint8_t bytes[] = {c.r, c.g, c.b};
                        file.write(reinterpret_cast<char*>(bytes), 3);
                    } else if constexpr (std::is_same_v<T, color16>) {
                        uint16_t comps[] = {c.r, c.g, c.b};
                        for (uint16_t comp : comps) {
                            file.put((comp >> 8) & 0xFF);
                            file.put(comp & 0xFF);
                        }
                    }
                }, pixel);
            }
        }
    }
};
