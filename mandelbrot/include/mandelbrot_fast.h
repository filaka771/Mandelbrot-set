#pragma once
#include <cstdint>
#include <vector>
#include <immintrin.h>

class mandelbrot_fast{

public:
    mandelbrot_fast(uint32_t x_resolution, uint32_t y_resolution)
    : resolution_(x_resolution, y_resolution),
    aligned_resolution_(x_resolution + (8 - (x_resolution % 8)), y_resolution)
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

        // Precalculated x coordinates, which 
        float* x_coords = (float*)_mm_malloc(aligned_resolution_.x_res * sizeof(float), 32);

        __m256 threshold = _mm256_set1_ps(4);

        for(uint32_t i = 0; i < aligned_resolution_.x_res; i ++) {
            x_coords[i] = viewport_.minReal + i * x_step;
        }

        // Main cycle
        for(uint32_t y = 0; y < aligned_resolution_.y_res; y ++) {
            float y_coord = viewport_.maxImag - y * y_step;
            __m256 y0_vec_half = _mm256_set1_ps(y_coord / 2);

            for(uint32_t x = 0; x < aligned_resolution_.x_res; x += 8) {
                __m256 x0_vec = _mm256_load_ps(&x_coords[x]);
                escape_times(x0_vec, y0_vec_half, 128, threshold);
            }
        }

        _mm_free(x_coords);
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

    // Vector contains num of iterations, which
    // required to verify that point lies or doesn't
    // lie in the Mandelbrot set

    float* mandelbrot_set_ = static_cast<float*>(_mm_malloc(aligned_resolution_.x_res * aligned_resolution_.y_res, sizeof(float)));

    __m256i escape_times(__m256 x0_vec, __m256 y0_vec_half, int max_iter, __m256 threshold){
        __m256 esc_mask = _mm256_set1_ps(-1.0f); // Mask for already escaped points
        __m256i esc_times_vec = _mm256_set1_epi32(0); // Escape times vector

        __m256 x_vec = x0_vec;
        __m256 y_vec = _mm256_mul_ps(y0_vec_half, _mm256_set1_ps(2.0f));

        for(int iter = 0; iter < max_iter; iter ++) {
            __m256 x_vec_next = _mm256_fmadd_ps(x_vec, x_vec, x0_vec); // x^2 + x_0
            x_vec_next = _mm256_fnmadd_ps(y_vec, y_vec, x_vec_next);   // x^2 + x_0 - y^2

            __m256 y_vec_next = _mm256_fmadd_ps(x_vec, y_vec, y0_vec_half); // x * y + (c_y / 2)
            y_vec_next = _mm256_mul_ps(y_vec_next, _mm256_set1_ps(2.0f));   // 2 * (x * y + (c_y / 2))

            x_vec = x_vec_next;
            y_vec = y_vec_next;

            if(iter % 8 == 0){
                __m256 norm2 = _mm256_fmadd_ps(x_vec, x_vec, _mm256_mul_ps(y_vec, y_vec));
                __m256 escaped_mask  = _mm256_cmp_ps(norm2, threshold, _CMP_GT_OS);

                // Record escape time for newly escaped points
                __m256i iter_vec = _mm256_set1_epi32(iter);
                esc_times_vec = _mm256_blendv_epi8(esc_times_vec, iter_vec, _mm256_castps_si256(esc_mask));
        
                // Update active mask
                esc_mask = _mm256_andnot_ps(esc_times_vec, esc_mask);
        
                // Early exit if ALL points escaped
                if(_mm256_testz_si256(_mm256_castps_si256(esc_mask), 
                                      _mm256_castps_si256(esc_mask))) {
                    break;
                }
            }
        }
        
        // Handle points, that never escaped
        __m256i max_iter_vec = _mm256_set1_epi32(max_iter);
        esc_times_vec = _mm256_blendv_epi8(esc_times_vec, max_iter_vec, _mm256_castps_si256(esc_mask));

        return esc_times_vec;
    };
};
