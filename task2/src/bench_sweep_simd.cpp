// bench_sweep_simd.cpp
#include "matmul.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

using Clock = std::chrono::high_resolution_clock;

// Runs fn() `reps` times, returns the median wall-clock time in seconds.
template <typename F>
double time_median(F&& fn, int reps) {
    std::vector<double> times(reps);
    for (int r = 0; r < reps; ++r) {
        auto start = Clock::now();
        fn();
        auto end = Clock::now();
        times[r] = std::chrono::duration<double>(end - start).count();
    }
    std::sort(times.begin(), times.end());
    return times[reps / 2];
}

int main() {
    // ---- Sweep configuration: edit these to fit your time budget ----
    std::vector<int> sizes = {64, 128, 256, 512, 1024};
    std::vector<const char*> widths = {"128", "256"};
    // Add "512" here only if your CPU/build actually supports AVX-512:
    // widths.push_back("512");

    const int REPS = 5;  // runs per config, we take the median

    printf("size,width,time_naive_s,time_simd_s,speedup\n");
    fflush(stdout);

    for (int size : sizes) {
        int M = size, N = size, K = size;
        int lda = K, ldb = K, ldc = N;

        std::vector<float> A(static_cast<size_t>(M) * lda);
        std::vector<float> B(static_cast<size_t>(N) * ldb);
        std::vector<float> C(static_cast<size_t>(M) * ldc);

        srand(42);
        for (auto& v : A) v = static_cast<float>(rand()) / RAND_MAX - 0.5f;
        for (auto& v : B) v = static_cast<float>(rand()) / RAND_MAX - 0.5f;

        fprintf(stderr, "size=%d : running naive...\n", size);
        double t_naive = time_median([&]() {
            matmul_naive(A.data(), B.data(), C.data(), M, N, K, lda, ldb, ldc);
        }, REPS);

        for (auto width : widths) {
            setenv("SIMD_WIDTH", width, 1);

            fprintf(stderr, "size=%d width=%s : running simd...\n", size, width);
            double t_simd = time_median([&]() {
                matmul_simd(A.data(), B.data(), C.data(), M, N, K, lda, ldb, ldc);
            }, REPS);

            double speedup = t_naive / t_simd;

            printf("%d,%s,%.6f,%.6f,%.4f\n",
                   size, width, t_naive, t_simd, speedup);
            fflush(stdout);
        }
    }

    return 0;
}