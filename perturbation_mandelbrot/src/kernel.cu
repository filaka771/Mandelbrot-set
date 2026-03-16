#include <cstdint>
#include <cuda_runtime.h>

#define GLITCH_FLAG -1
#define ESCAPE_RADIUS 4.0
#define GLITCH_THRESHOLD 1e-10

// Cuda kernel
__global__ void escape_times_kernel(
    const double2* __restrict__ reference_orbits,
    int* __restrict__ escape_times,
    double2 delta_c,
    int width, int height, int max_iter,
    double2 c_ref,
    double pixel_scale
) {

    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;

    if(x >= width || y >= height)
        return;

    const int idx = y * width + x;

    // Calculate pixel's complex coordinate and delta

    const double pixel_re = c_ref.x + (x - width * 0.5) * pixel_scale;
    const double pixel_im = c_ref.y - (y - height * 0.5) * pixel_scale;

    // delta_c = c_pixel - c_ref
    const double2 delta_c_pixel = make_double2(pixel_re - c_ref.x, pixel_im - c_ref.y);

    // Perturbation loop
    double2 delta = make_double2(0.0, 0.0);
    int iterations = max_iter;
    bool is_glitch = false;

    for(int iter = 0; iter < max_iter; iter ++){
        const double2 z_ref = reference_orbits[iter];

        // Perturbation formula
        // delta_{n + 1} = 2 * z_n * delta_n + delta_n ^ 2 + delta_c

        // calculate complex delta_n ^ 2
        const double2 delta_sqr = make_double2(delta.x * delta.x - delta.y * delta.y,
                                               2.0 * delta.x * delta.y);

        // complex multiply 2 * z_n * delta_n
        const double2 term_2zd = make_double2(2.0 * (z_ref.x * delta.x - z_ref.y * delta.y),
                        2.0 * (z_ref.x * delta.y + z_ref.y * delta.x));

        // calculate delta_{n + 1}
        delta.x = term_2zd.x + delta_sqr.x + delta_c_pixel.x;
        delta.y = term_2zd.y + delta_sqr.y + delta_c_pixel.y;

        // Escape test
        const double2 z_total = make_double2(z_ref.x + delta.x, z_ref.y + delta.y);
        const double escape_test = z_total.x * z_total.x + z_total.y * z_total.y;

        if(escape_test >= ESCAPE_RADIUS) {
            iterations = iter;
            break;
        }

        // Glitch detection
        // glitch if |delta|/|z| > threshold
        const double delta_norm_sqr = delta.x * delta.x + delta.y * delta.y;
        const double z_norm_sqr     = z_ref.x * z_ref.x + z_ref.y * z_ref.y;

        if(delta_norm_sqr > (GLITCH_THRESHOLD * GLITCH_THRESHOLD) * z_norm_sqr) {
            is_glitch = true;
            break;
        }
    }
    // Write result
    escape_times[idx] = is_glitch ? GLITCH_FLAG : iterations;
}

// Kernel launch wrapper
cudaError_t launch_mandelbrot_kernel(
    int* d_output,
    const double2* d_reference_orbit,
    int width, int height, int max_iter,
    double c_ref_x, double c_ref_y,
    double pixel_scale
) {
    const dim3 block_dim(16, 16);
    const dim3 grid_dim(
        (width + block_dim.x - 1) / block_dim.x,
        (height + block_dim.y - 1)/ block_dim.y
    );

    // Launch kernel
    escape_times_kernel<<<grid_dim, block_dim>>>(
        d_reference_orbit,
        d_output,
        make_double2(0.0, 0.0),  // delta_c placeholder (computed per-pixel in kernel)
        width, height, max_iter,
        make_double2(c_ref_x, c_ref_y),
        pixel_scale
    );

    cudaError_t launch_err = cudaGetLastError();

    if (launch_err != cudaSuccess) return
        launch_err;
    
    return cudaDeviceSynchronize();
}
