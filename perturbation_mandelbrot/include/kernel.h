#pragma once
#include <cuda_runtime.h>

// Constants (shared between host and device)
#define GLITCH_FLAG -1
#define GLITCH_THRESHOLD 1e-10

// Error checking macro
#define cudaCheck(err) __cuda_check_impl(err, __FILE__, __LINE__)
inline void __cuda_check_impl(cudaError_t err, const char* file, int line) {
    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA ERROR [%s:%d]: %s\n", file, line, cudaGetErrorString(err));
        exit(err);
    }
}

// Kernel launch wrapper (defined in kernel.cu)
cudaError_t launch_mandelbrot_kernel(
    int* d_output,                      // [width*height] output buffer
    const double2* d_reference_orbit,   // [max_iter] reference orbit
    int width, int height, int max_iter,
    double c_ref_x, double c_ref_y,     // Reference point (complex)
    double pixel_scale                  // Complex units per pixel
);
