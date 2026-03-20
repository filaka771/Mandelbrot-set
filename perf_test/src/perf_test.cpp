#include <chrono>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>
#include <string>

#include "cli_table.h"
#include "mandelbrot.h"
#include "mandelbrot_fast.h"

#define NUM_OF_ITERATIONS 20

int main(){
    auto naive_mandelbrot = mandelbrot<float>(3840, 2160);
    naive_mandelbrot.set_viewport(-2.5, 1, -1.25, 1.25);

    auto fast_mandelbrot = mandelbrot_fast(3840, 2160);
    fast_mandelbrot.set_viewport(-2.5, 1, -1.25, 1.25);

    // ---- Perf test for naive Mandelbrot ----
    naive_mandelbrot.render(); // Cache heating
    auto start = std::chrono::steady_clock::now();

    for(int i = 0; i < NUM_OF_ITERATIONS; i ++) {
        naive_mandelbrot.render();
    }

    auto end = std::chrono::steady_clock::now();
    auto naive_exec_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double naive_exec_time_d = naive_exec_time.count() / NUM_OF_ITERATIONS / 1000.0;

    // ---- Perf test for vectorized Mandelbrot ----
    fast_mandelbrot.render(); // Cache heating
    start = std::chrono::steady_clock::now();

    for(int i = 0; i < NUM_OF_ITERATIONS; i ++) {
        fast_mandelbrot.render();
    }

    end = std::chrono::steady_clock::now();
    auto fast_exec_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double fast_exec_time_d = fast_exec_time.count() / NUM_OF_ITERATIONS / 1000.0;

    std::vector<std::vector<std::string>> perf_table_content = {
        {"Naive Mandelbrot performance", std::to_string(naive_exec_time_d) + " s"},
        {"Vectorized Mandelbrot performance", std::to_string(fast_exec_time_d) + " s"}
    };

    std::string perf_table_title = "Time of rendering one 4K frame of Mandelbrot set";

    auto perf_table = cli_table::table(perf_table_content, perf_table_title);
    perf_table.print();

    return 0;
}
