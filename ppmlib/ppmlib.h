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
    struct pixel{
        uint32_t x, y;
        pixel(uint32_t x_coord, uint32_t y_coord)
        : x(x_coord), y(y_coord){}
    };

    using color = std::variant<color8, color16>;

    ppmimg(uint32_t w, uint32_t h, uint32_t cd):
                  width_(w), height_(h), col_depth_(cd){
        if(col_depth_ <= 255){
            pixel_list_.resize(width_ * height_, color8(0,0,0));
        }

        if(255 < col_depth_ && col_depth_ <= 255 * 255){
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

    void set_pixel(pixel p, color col) {
        if (p.x >= width_ || p.y >= height_)
            throw std::out_of_range("Pixel coordinates out of bounds");

        size_t idx = p.y * width_ + p.x;

        bool type_ok = std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, color8>)
                return col_depth_ <= 255;
            else if constexpr (std::is_same_v<T, color16>)
                return col_depth_ > 255;
            else
                return false;
        }, col);

        if (!type_ok)
            throw std::invalid_argument("Color type does not match image depth");

        pixel_list_[idx] = col;
    }

    void save(const char* filename, bool bin){
        if(bin) 
            save_p6(filename);
        else
            save_p3(filename);
    }

private:

    void save_p3(const char* filename){
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file");
        }

        // Write header
        file << "P3\n";
        file << width_ << ' ' <<height_ << "\n";
        file << col_depth_ << "\n";

        // Write pixels colors
        for(uint32_t y = 0; y < height_; y ++){
            for(uint32_t x = 0; x < width_; x ++){
                const auto& pixel = pixel_list_[y * width_ + x];
                std::visit([&file](const auto color){
                    file << static_cast<int>(color.r) << ' '
                    << static_cast<int>(color.g) << ' '
                    << static_cast<int>(color.b);
                }, pixel);
                file << "\n";
            }
        }
    }

    void save_p6(const char* filename){
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file for writing");
        }

        file << "P6\n";
        file << width_ << ' ' << height_ << '\n';
        file << col_depth_ << '\n';

        // Write pixels colors
        for (uint32_t y = 0; y < height_; ++y) {
            for (uint32_t x = 0; x < width_; ++x) {
                const auto& pixel = pixel_list_[y * width_ + x];

                std::visit([&file](const auto& color) {
                    using T = std::decay_t<decltype(color)>;

                    if constexpr (std::is_same_v<T, color8>) {
                        // 8 bit 
                        uint8_t bytes[] = {color.r, color.g, color.b};
                        file.write(reinterpret_cast<char*>(bytes), 3);
                    }
                    else if constexpr (std::is_same_v<T, color16>) {
                        // 16 bit
                        uint16_t components[] = {color.r, color.g, color.b};
                        for (uint16_t comp : components) {
                            uint8_t high = (comp >> 8) & 0xFF;
                            uint8_t low  = comp & 0xFF;
                            file.put(high);
                            file.put(low);
                        }
                    }
                }, pixel);
            }
        }
    }
};
