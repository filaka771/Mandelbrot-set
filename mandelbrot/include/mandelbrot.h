#pragma once
#include <cstdint>
#include <vector>

template<typename T>
class mdb{

public:
    mdb(uint32_t x_resolution, uint32_t y_resolution)
    : resolution({x_resolution, y_resolution}) {
        mandelbrot_set_.resize(resolution_.x_res * resolution_.y_res);
    };

    void set_viewport(T min_real, T max_real, T min_imag, T max_imag)
    : viewport_({min_real, max_real, min_imag, max_imag}) {};

private:
    struct complex{
        T x, y;

        complex(T real, T imag)
        : x(real), y(imag) {};

        T abs(complex z){
            return abs(z.x * z.x + z.y * z.y);
        };

        complex operator+ (const complex z_1, const complex z_2){
            complex z;
            z.x = z_1.x + z_2.x;
            z.y = z_1.y + z_2.y;
            return z;
        };

        complex operator* (const complex z_1, const complex z_2){
            complex z;
            z.x = z_1.x * z_2.x - z_1.y * z_2.y;
            z.y = z_1.x * z_2.y + z_1.y * z_2.x;

            return z;
        };

        complex operator= ()
    };

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

    int escape_time(complex z_0, int n_iter, T treshold){
        int iter = 0;
        complex z = z_0;
        while(iter < n_iter){
            z = z * z + z_0;
        }
    };


};
