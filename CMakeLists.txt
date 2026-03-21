cmake_minimum_required(VERSION 3.10)
project(Mandlebrot)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenMP REQUIRED) 

# Define include paths
set(PPMLIB_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ppmlib/include)
set(SHADERS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/include)
set(MANDELBROT_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/mandelbrot/include)
set(MULTIPRECISION ${CMAKE_CURRENT_SOURCE_DIR}/multiprecision/include)
set(CLI_TABLE ${CMAKE_CURRENT_SOURCE_DIR}/perf_test/include)

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
target_link_libraries(mandelbrot PRIVATE OpenMP::OpenMP_CXX)

target_compile_options(mandelbrot PRIVATE -O3)

# 4. Fast Mandelbrot Executable
add_executable(mandelbrot_fast mandelbrot/src/mandelbrot_fast.cpp)
target_include_directories(mandelbrot_fast PRIVATE 
  ${MANDELBROT_INCLUDE}
  ${SHADERS_INCLUDE}
  ${PPMLIB_INCLUDE}
)
target_link_libraries(mandelbrot_fast PRIVATE shaders)
target_compile_options(mandelbrot_fast PRIVATE -mavx2 -mfma -O3)

# 5. Mandelbrot multiprecision
add_executable(multi_mandelbrot mandelbrot/src/mandelbrot_multiprecision.cpp)
target_include_directories(multi_mandelbrot PRIVATE
  ${MULTIPRECISION}
  ${MANDELBROT_INCLUDE}
  ${SHADERS_INCLUDE}
  ${PPMLIB_INCLUDE}
)

target_link_libraries(multi_mandelbrot PRIVATE
  OpenMP::OpenMP_CXX
  shaders
  mpfr  # library name as known to the linker
  gmp   # MPFR depends on GMP, so link both
)

target_compile_options(multi_mandelbrot PRIVATE -O3)
# 6. Performance test
add_executable(perf_test perf_test/src/perf_test.cpp)
target_include_directories(perf_test PRIVATE
    ${CLI_TABLE}
    ${MANDELBROT_INCLUDE}
    ${SHADERS_INCLUDE}
    ${PPMLIB_INCLUDE}
)
target_link_libraries(multi_mandelbrot PRIVATE shaders OpenMP::OpenMP_CXX)
target_compile_options(perf_test PRIVATE -mavx2 -mfma -O1)
