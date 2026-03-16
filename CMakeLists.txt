cmake_minimum_required(VERSION 3.10)
project(MANDELBROT_SET LANGUAGES CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES "61")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Define include paths
set(PPMLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ppmlib/include)
set(SHADERS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/include)
set(MANDELBROT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/mandelbrot/include)
set(MULTIPRECISION ${CMAKE_CURRENT_SOURCE_DIR}/multiprecision/include)

  # 1. Shaders Library 
add_library(shaders STATIC shaders/src/shaders.cpp)
target_include_directories(shaders PUBLIC ${SHADERS_INCLUDE} ${PPMLIB_INCLUDE})

# 2. Gradient Executable
add_executable(gradient ppmlib/example/gradient.cpp)
target_include_directories(gradient PRIVATE ${PPMLIB_INCLUDE})

# 3. Mandelbrot Executable
add_executable(mandelbrot mandelbrot/src/mandelbrot.cpp)
target_include_directories(mandelbrot PRIVATE 
  ${MANDELBROT_INCLUDE}
  ${SHADERS_INCLUDE}
  ${PPMLIB_INCLUDE}
)
target_link_libraries(mandelbrot PRIVATE shaders)

# 4. Fast Mandelbrot Executable
add_executable(mandelbrot_fast mandelbrot/src/mandelbrot_fast.cpp)
target_include_directories(mandelbrot_fast PRIVATE 
  ${MANDELBROT_INCLUDE}
  ${SHADERS_INCLUDE}
  ${PPMLIB_INCLUDE}
)
target_link_libraries(mandelbrot_fast PRIVATE shaders)
target_compile_options(mandelbrot_fast PRIVATE -mavx2 -mfma)

# 5. Mandelbrot multiprecision
add_executable(multi_mandelbrot mandelbrot/src/mandelbrot_multiprecision.cpp)
target_include_directories(multi_mandelbrot PRIVATE
  ${MULTIPRECISION}
  ${MANDELBROT_INCLUDE}
  ${SHADERS_INCLUDE}
  ${PPMLIB_INCLUDE}
)

target_link_libraries(multi_mandelbrot PRIVATE
  shaders
  mpfr  # library name as known to the linker
  gmp   # MPFR depends on GMP, so link both
)


# ── GPU Mandelbrot Executable (Perturbation) ──
add_library(kernel ${CMAKE_CURRENT_SOURCE_DIR}/perturbation_mandelbrot/src/kernel.cu)
add_executable(multi_mandelbrot_gpu
  perturbation_mandelbrot/src/main_gpu.cpp          # Your main() that uses mdb<T>::renderGPU()
)

find_package(CUDAToolkit REQUIRED)
set(PERTURBATION_MANDELBROT ${CMAKE_CURRENT_SOURCE_DIR}/perturbation_mandelbrot/include)
target_include_directories(multi_mandelbrot_gpu PRIVATE
  ${PERTURBATION_MANDELBROT}
  ${SHADERS_INCLUDE} # Ok
  ${PPMLIB_INCLUDE}  # Ok
)

target_link_libraries(multi_mandelbrot_gpu PRIVATE shaders CUDA::cudart kernel)

# CUDA-specific flags
set_target_properties(multi_mandelbrot_gpu PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
)

target_compile_options(multi_mandelbrot_gpu PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:
--use_fast_math
-Xcudafe=--display_error_number
>
)

# Optional: Enable debug symbols for cuda-gdb
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#target_compile_options(multi_mandelbrot_gpu PRIVATE
#$<$<COMPILE_LANGUAGE:CUDA>:-g -G>
#)
#endif())
