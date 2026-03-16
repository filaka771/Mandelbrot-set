#pragma once
#include <cstdint>
#include <complex>
#include <vector>
#include <cmath>
#include <cuda_runtime.h>
#include "shaders.h"
#include "ppmlib.h"
#include "kernel.h"  // Declares launchMandelbrotKernel, GLITCH_FLAG, cudaCheck

// ============================================================================
// Helper: Convert custom complex<T> to CUDA double2
// ============================================================================
template<typename T>
static inline double2 to_double2(const std::complex<T>& c) {
    return make_double2(static_cast<double>(c.real()), static_cast<double>(c.imag()));
}

template<typename T>
static inline double2 to_double2(T real, T imag) {
    return make_double2(static_cast<double>(real), static_cast<double>(imag));
}

// ============================================================================
// Mandelbrot Renderer Class (GPU Perturbation Support)
// ============================================================================
template<typename T>
class mdb {
public:
    using Complex = std::complex<T>;  // Use standard complex for clarity

    mdb(uint32_t x_resolution, uint32_t y_resolution)
    : resolution_{x_resolution, y_resolution} {
        mandelbrot_set_.resize(resolution_.x_res * resolution_.y_res);
    }

    // Set viewport in complex plane coordinates
    void set_viewport(T min_real, T max_real, T min_imag, T max_imag) {
        viewport_ = {min_real, max_real, min_imag, max_imag};
    }

    // Set viewport using center + scale (more intuitive for zooming)
    void set_viewport_center(Complex center, T scale) {
        T half_width = scale * static_cast<T>(resolution_.x_res) / 2;
        T half_height = scale * static_cast<T>(resolution_.y_res) / 2;
        viewport_ = {
            center.real() - half_width,
            center.real() + half_width,
            center.imag() - half_height,
            center.imag() + half_height
        };
    }

    /**
     * @brief Render Mandelbrot set using GPU perturbation theory
     * 
     * Uses double-precision perturbation for speed, with optional 
     * high-precision glitch correction.
     */
    void renderGPU(int max_iter = 1000, T escape_radius = T(2.0)) {
        // ── Step 1: Compute Reference Orbit (CPU, double precision) ──
        const Complex c_ref = get_center();
        std::vector<double2> reference_orbit = compute_reference_orbit_double(c_ref, max_iter);
        
        // ── Step 2: Allocate GPU Memory ──
        double2* d_reference_orbit = nullptr;
        int* d_output = nullptr;
        
        cudaCheck(cudaMalloc(&d_reference_orbit, max_iter * sizeof(double2)));
        cudaCheck(cudaMalloc(&d_output, resolution_.x_res * resolution_.y_res * sizeof(int)));
        
        // ── Step 3: Upload Reference Orbit ──
        cudaCheck(cudaMemcpy(d_reference_orbit, reference_orbit.data(),
                             max_iter * sizeof(double2),
                             cudaMemcpyHostToDevice));
        
        // ── Step 4: Calculate Pixel Scale ──
        // Complex units per pixel (for coordinate mapping in kernel)
        const double pixel_scale = static_cast<double>(viewport_.maxReal - viewport_.minReal) 
                                   / static_cast<double>(resolution_.x_res);
        
        // ── Step 5: Launch CUDA Kernel ──
        cudaCheck(launch_mandelbrot_kernel(
                                           d_output,
                                           d_reference_orbit,
                                           static_cast<int>(resolution_.x_res),
                                           static_cast<int>(resolution_.y_res),
                                           max_iter,
                                           static_cast<double>(c_ref.real()),
                                           static_cast<double>(c_ref.imag()),
                                           pixel_scale
                                        ));
        
        // ── Step 6: Download Results ──
        std::vector<int> gpu_results(resolution_.x_res * resolution_.y_res);
        cudaCheck(cudaMemcpy(gpu_results.data(), d_output,
                             resolution_.x_res * resolution_.y_res * sizeof(int),
                             cudaMemcpyDeviceToHost));
        
        // ── Step 7: Glitch Correction (CPU, high precision) ──
        int glitch_count = 0;
        const double escape_sq = static_cast<double>(escape_radius * escape_radius);
        
        for (uint32_t y = 0; y < resolution_.y_res; ++y) {
            for (uint32_t x = 0; x < resolution_.x_res; ++x) {
                const int idx = y * resolution_.x_res + x;
                
                if (gpu_results[idx] == GLITCH_FLAG) {
                    ++glitch_count;
                    
                    // Calculate pixel's complex coordinate
                    const T c_real = viewport_.minReal + static_cast<T>(x) * get_pixel_step_real();
                    const T c_imag = viewport_.maxImag - static_cast<T>(y) * get_pixel_step_imag(); // Y inverted
                    const Complex c_pixel(c_real, c_imag);
                    
                    // Recompute with full T-precision
                    mandelbrot_set_[idx] = compute_pixel_iterations(c_pixel, max_iter, escape_sq);
                } else {
                    mandelbrot_set_[idx] = gpu_results[idx];
                }
            }
        }
        
        // ── Step 8: Cleanup GPU Memory ──
        cudaFree(d_reference_orbit);
        cudaFree(d_output);
        
        // Report statistics
        const float glitch_pct = 100.0f * glitch_count / (resolution_.x_res * resolution_.y_res);
        printf("[GPU Render] %ux%u, iter=%d, glitches=%d (%.2f%%)\n",
               resolution_.x_res, resolution_.y_res, max_iter, glitch_count, glitch_pct);
    }

    /**
     * @brief Render using pure CPU (fallback or verification)
     */
    void renderCPU(int max_iter = 1000, T escape_radius = T(2.0)) {
        const double escape_sq = static_cast<double>(escape_radius * escape_radius);
        
        for (uint32_t y = 0; y < resolution_.y_res; ++y) {
            for (uint32_t x = 0; x < resolution_.x_res; ++x) {
                const T c_real = viewport_.minReal + static_cast<T>(x) * get_pixel_step_real();
                const T c_imag = viewport_.maxImag - static_cast<T>(y) * get_pixel_step_imag();
                const Complex c(c_real, c_imag);
                
                mandelbrot_set_[y * resolution_.x_res + x] = 
                    compute_pixel_iterations(c, max_iter, escape_sq);
            }
        }
    }

    // Save rendered image using shader function
    template<auto shader>
    void save_image(const char* file_name, int color_depth = 255) {
        ppmimg image(resolution_.x_res, resolution_.y_res, color_depth);

        for (uint32_t y = 0; y < resolution_.y_res; ++y) {
            for (uint32_t x = 0; x < resolution_.x_res; ++x) {
                const int iterations = mandelbrot_set_[y * resolution_.x_res + x];
                const ppmimg::pixel pixel(x, y);
                const ppmimg::color color = shader(iterations, color_depth);
                image.set_pixel(pixel, color);
            }
        }
        image.save(file_name, true);
    }

    // Access rendered data
    const std::vector<int>& get_iterations() const { return mandelbrot_set_; }
    int get_iterations(uint32_t x, uint32_t y) const { 
        return mandelbrot_set_[y * resolution_.x_res + x]; 
    }

private:
    // ── Data Structures ──
    struct Resolution {
        uint32_t x_res, y_res;
        Resolution(uint32_t x, uint32_t y) : x_res(x), y_res(y) {}
    };

    struct Viewport {
        T minReal, maxReal, minImag, maxImag;
        Viewport(T min_r, T max_r, T min_i, T max_i) 
        : minReal(min_r), maxReal(max_r), minImag(min_i), maxImag(max_i) {}
        Viewport() : minReal(0), maxReal(0), minImag(0), maxImag(0) {}
    };

    Resolution resolution_;
    Viewport viewport_;
    std::vector<int> mandelbrot_set_;  // Iteration counts per pixel

    // ── Helper: Pixel Step Sizes ──
    T get_pixel_step_real() const {
        return (viewport_.maxReal - viewport_.minReal) / static_cast<T>(resolution_.x_res);
    }
    
    T get_pixel_step_imag() const {
        return (viewport_.maxImag - viewport_.minImag) / static_cast<T>(resolution_.y_res);
    }
    
    Complex get_center() const {
        return Complex(
                       (viewport_.minReal + viewport_.maxReal) / T(2),
                       (viewport_.minImag + viewport_.maxImag) / T(2)
                       );
    }

    // ── Helper: Compute Reference Orbit (double precision for GPU) ──
    std::vector<double2> compute_reference_orbit_double(Complex c_ref, int max_iter) {
        std::vector<double2> orbit(max_iter);
        Complex z(0, 0);
        
        for (int n = 0; n < max_iter; ++n) {
            orbit[n] = make_double2(
                                    static_cast<double>(z.real()),
                                    static_cast<double>(z.imag())
                                    );
            z = z * z + c_ref;
        }
        return orbit;
    }

    // ── Helper: Single Pixel Iteration (high precision) ──
    int compute_pixel_iterations(Complex c, int max_iter, double escape_sq) {
        Complex z(0, 0);
        
        for (int n = 0; n < max_iter; ++n) {
            // |z|^2 = z.real()^2 + z.imag()^2
            const double norm_sq = static_cast<double>(z.real() * z.real() + z.imag() * z.imag());
            if (norm_sq > escape_sq) {
                return n;
            }
            z = z * z + c;
        }
        return max_iter;
    }
};
