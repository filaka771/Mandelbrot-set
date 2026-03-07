#pragma once
#include <cstdint>
#include <immintrin.h>

#include "ppmlib.h"
#include "shaders.h"

class mandelbrot_fast{

public:
    mandelbrot_fast(uint32_t x_resolution, uint32_t y_resolution)
    : resolution_(x_resolution, y_resolution),
    aligned_resolution_((x_resolution + 7) & ~7, y_resolution)
    {};

    ~mandelbrot_fast(){
        _mm_free(mandelbrot_set_);
    };

    void set_viewport(float min_real, float max_real, float min_imag, float max_imag) {
        viewport_ = {min_real, max_real, min_imag, max_imag};
    };

    void render(){
        float x_step = (viewport_.maxReal - viewport_.minReal) / static_cast<float>(resolution_.x_res);
        float y_step = (viewport_.maxImag - viewport_.minImag) / static_cast<float>(resolution_.y_res);

        __m256 threshold = _mm256_set1_ps(4);

        // Precalculated x coordinates
        float* x_coords = (float*)_mm_malloc(aligned_resolution_.x_res * sizeof(float), 32);


        for(uint32_t i = 0; i < aligned_resolution_.x_res; i ++) {
            x_coords[i] = viewport_.minReal + i * x_step;
        }

        // Main cycle
        for(uint32_t y = 0; y < aligned_resolution_.y_res; y ++) {
            float y_coord = viewport_.maxImag - y * y_step;
            __m256 y0_vec_half = _mm256_set1_ps(y_coord / 2);

            for(uint32_t x = 0; x < aligned_resolution_.x_res; x += 8) {
                __m256 x0_vec = _mm256_load_ps(&x_coords[x]);
                __m256i escape_times_vec = escape_times(x0_vec, y0_vec_half, 128, threshold);

                _mm256_store_si256((__m256i*)(mandelbrot_set_ + y * aligned_resolution_.x_res + x), escape_times_vec);
            }
        }

        _mm_free(x_coords);
    };

    void save_image(const char* file_name){
        ppmimg image(resolution_.x_res, resolution_.y_res, 255);

        for(int y = 0; y < resolution_.y_res; y ++) {
            for(int x = 0; x < aligned_resolution_.x_res; x ++) {
                if(x >= resolution_.x_res)
                    continue;

                ppmimg::pixel pixel (x, y);
                ppmimg::color color = grey(mandelbrot_set_[y * aligned_resolution_.x_res + x], 128);

                image.set_pixel(pixel, color);

            }
        }

        image.save(file_name,false);
    };

private:
    struct resolution {
        uint32_t x_res, y_res;

        resolution(uint32_t x_resolution, uint32_t y_resolution)
        : x_res(x_resolution), y_res(y_resolution) {};
    };

    struct viewport {
        float minReal, maxReal, minImag, maxImag;

        viewport(float min_real, float max_real, float min_imag, float max_imag)
        : minReal(min_real), maxReal(max_real),
        minImag(min_imag), maxImag(max_imag){};
    };

    resolution resolution_;
    resolution aligned_resolution_;

    viewport viewport_ = {0, 0, 0, 0};

    int* mandelbrot_set_ = static_cast<int*>(_mm_malloc(aligned_resolution_.x_res * aligned_resolution_.y_res * sizeof(int), 32));

    __m256i escape_times(__m256 x0_vec, __m256 y0_vec_half, int max_iter, __m256 threshold) __attribute__((always_inline)) {
        __m256 active = _mm256_castsi256_ps(_mm256_set1_epi32(-1));   // mask of not already escaped
        __m256i esc_times = _mm256_set1_epi32(0);                       // escape times

        __m256 x = x0_vec;
        __m256 y = _mm256_mul_ps(y0_vec_half, _mm256_set1_ps(2.0f));

        for (int iter = 0; iter < max_iter; ++iter) {
            // check every 4th iteration
            if (iter % 4 == 0) {
                __m256 norm2 = _mm256_fmadd_ps(x, x, _mm256_mul_ps(y, y));
                __m256 escaped = _mm256_cmp_ps(norm2, threshold, _CMP_GT_OS); // !!!Does it returns __m256 or true/false

                // newly escaped = still active AND just escaped
                __m256 new_escaped = _mm256_and_ps(active, escaped);

                // record escape time for newly escaped points (using current iter)
                __m256i iter_vec = _mm256_set1_epi32(iter);
                esc_times = _mm256_blendv_epi8(esc_times, iter_vec,
                                               _mm256_castps_si256(new_escaped));

                // update active mask: remove newly escaped points
                active = _mm256_andnot_ps(new_escaped, active);

                // early exit if all points have escaped
                if (_mm256_testz_ps(active, active)) {
                    break;
                }
            }

            __m256 x_next = _mm256_fmadd_ps(x, x, x0_vec);
            x_next = _mm256_fnmadd_ps(y, y, x_next);

            __m256 y_next = _mm256_fmadd_ps(x, y, y0_vec_half);
            y_next = _mm256_mul_ps(y_next, _mm256_set1_ps(2.0f));

            x = _mm256_blendv_ps(x, x_next, active);  // Only update if active=1
            y = _mm256_blendv_ps(y, y_next, active);  // Freeze escaped points
        }

        // points that never escaped get max_iter
        __m256i max_iter_vec = _mm256_set1_epi32(max_iter);
        esc_times = _mm256_blendv_epi8(esc_times, max_iter_vec,
                                       _mm256_castps_si256(active));

    return esc_times;
    };
};
