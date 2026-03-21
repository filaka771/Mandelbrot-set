#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include "ppmlib.h"
#include <omp.h>

template<typename T>
class mandelbrot{
public:
    mandelbrot(uint32_t x_resolution, uint32_t y_resolution)
    : resolution_(x_resolution, y_resolution) {
        mandelbrot_set_.resize(resolution_.x_res * resolution_.y_res);
    };

    void set_viewport(T min_real, T max_real, T min_imag, T max_imag) {
        viewport_ = {min_real, max_real, min_imag, max_imag};
    };

    void set_calculation_iterations(int calculation_iterations) {
        calculation_iterations_ = calculation_iterations;
    }

    void render(bool parallel = false) {
        T x_step = (viewport_.maxReal - viewport_.minReal) / static_cast<T>(resolution_.x_res);
        T y_step = (viewport_.maxImag - viewport_.minImag) / static_cast<T>(resolution_.y_res);


        if (parallel) {
            #pragma omp parallel for num_threads(12) collapse(2) schedule(dynamic)
            for (uint32_t y = 0; y < resolution_.y_res; ++y) {
                for (uint32_t x = 0; x < resolution_.x_res; ++x) {
                    T real = viewport_.minReal + static_cast<T>(x) * x_step;
                    T imag = viewport_.maxImag - static_cast<T>(y) * y_step;
                    complex c(real, imag);
                    mandelbrot_set_[y * resolution_.x_res + x] = escape_time(c, calculation_iterations_, 4);
                }
                //std::cout << "Processed " << y << " lines from " << resolution_.y_res << " lines.\n";
            }
        } else {
            // Sequential version – same pixel‑wise computation
            for (uint32_t y = 0; y < resolution_.y_res; ++y) {
                for (uint32_t x = 0; x < resolution_.x_res; ++x) {
                    T real = viewport_.minReal + static_cast<T>(x) * x_step;
                    T imag = viewport_.maxImag - static_cast<T>(y) * y_step;
                    complex c(real, imag);
                    mandelbrot_set_[y * resolution_.x_res + x] = escape_time(c, calculation_iterations_, 4);
                }
            }
        }
    }

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
    int calculation_iterations_ = 128;

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
