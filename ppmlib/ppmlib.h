#pragma once
#include <vector>
#include <cstdint>
#include <variant>
#include <stdexcept>

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

    // color - alias for current type of pixel_list_ nodes
    // color must be public interface to use it for working with pictures

    // pixel - struct which would contain x and y coordinates of pixel on
    // image and would be public, to make it used by user.
    // pixel constructor would verify, that x and y less then width_ and
    // height_. 

    uint32_t width() const {
        return width_;
    }

    uint32_t height() const {
        return height_;
    }

    uint32_t col_depth() const{
        return col_depth_;
    }

    void set_pixel(pixel pixel, color color){

    }
};
