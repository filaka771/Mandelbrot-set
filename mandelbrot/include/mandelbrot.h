#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include "ppmlib.h"

template<typename T>
class mdb{

public:
    mdb(uint32_t x_resolution, uint32_t y_resolution)
        : resolution_(x_resolution, y_resolution) {
        mandelbrot_set_.resize(resolution_.x_res * resolution_.y_res);
    };

    void set_viewport(T min_real, T max_real, T min_imag, T max_imag) {
        viewport_ = {min_real, max_real, min_imag, max_imag};
    };

    void render(){
        T x_step = (viewport_.maxReal - viewport_.minReal) / static_cast<T>(resolution_.x_res);
        T y_step = (viewport_.maxImag - viewport_.minImag) / static_cast<T>(resolution_.y_res);

        complex z_0 = {viewport_.minReal, viewport_.maxImag};

        for(uint32_t y = 0; y < resolution_.y_res; y ++){
            for(uint32_t x = 0; x < resolution_.x_res; x ++){
                mandelbrot_set_[y * resolution_.x_res + x] = escape_time(z_0, 128, 4);
                z_0.x += x_step;
            }
            z_0.x = viewport_.minReal;
            z_0.y -= y_step;
        }
    };

    template<auto shader>
    void save_image(const char* file_name, const int color_depth){
        ppmimg image(resolution_.x_res, resolution_.y_res, 255);

        for(uint32_t y = 0; y < resolution_.y_res; y ++){
            for(uint32_t x = 0; x < resolution_.x_res; x ++){
                ppmimg::pixel pixel(x, y);
                ppmimg::color color = shader(mandelbrot_set_[y * resolution_.x_res + x], color_depth);
                image.set_pixel(pixel, color);
            }
        }

        image.save(file_name, true);
    };

private:
    struct resolution{
        uint32_t x_res, y_res;

        resolution(uint32_t x_resolution, uint32_t y_resolution)
            : x_res(x_resolution), y_res(y_resolution) {};
    };

    struct viewport{
        T minReal, maxReal, minImag, maxImag;

        viewport(T min_real, T max_real, T min_imag, T max_imag)
            : minReal(min_real), maxReal(max_real),
              minImag(min_imag), maxImag(max_imag){};
    };

    resolution resolution_;
    viewport viewport_ = {0, 0, 0, 0};

    // Vector contains num of iterations, which
    // required to verify that point lies or doesn't
    // lie in the Mandelbrot set
    std::vector<int> mandelbrot_set_;

    struct complex{
        T x, y;

        complex(T real = T(), T imag = T())
            : x(real), y(imag) {};

        T norm2() const{
            return x * x + y * y;
        };

        // assignment operators (must be members)
        complex& operator=(const complex& other) {
            x = other.x;
            y = other.y;
            return *this;
        };

        // compound assignment: can be members
        complex& operator+=(const complex& other) {
            x += other.x;
            y += other.y;
            return *this;
        };

        complex& operator*=(const complex& other) {
            T new_x = x * other.x - y * other.y;
            y = x * other.y + y * other.x;
            x = new_x;
            return *this;
        };
    };

    friend complex operator+(const complex& a, const complex& b) {
        return complex(a.x + b.x, a.y + b.y);
    };

    friend complex operator*(const complex& a, const complex& b) {
        return complex(a.x * b.x - a.y * b.y,
                       a.x * b.y + a.y * b.x);
    };


    int escape_time(complex z_0, int max_iter, T treshold){
        int iter = 0;
        complex z = z_0;
        while(iter < max_iter){
            z = z * z + z_0;
            if(z.norm2() > treshold)
                return iter;
            iter ++;
        }
        return max_iter;
    };
};
