#include <cstdio>
#include <cstdlib>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: traversing a 2D array in row-major order (the way it's
 * actually laid out in memory) is drastically faster than column-major,
 * because CPUs are good at scanning contiguous memory and bad at jumping.
 
 * Sweeps four sizes chosen to sit inside L1D, L2, L3, and beyond L3
 * into main memory (sizes below are tuned for this machine: AMD Zen3,
 * L1D 32KB/core, L2 512KB/core, L3 16MB shared -- recompute for yours
 * if you're on different hardware).
 */

static long long traverse_row_major(const std::vector<int>& a, int n) {
    long long sum = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            sum += a[i * n + j];
    return sum;
}

static long long traverse_column_major(const std::vector<int>& a, int n) {
    long long sum = 0;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            sum += a[i * n + j];
    return sum;
}

struct SizeClass {
    const char* label;
    int n;
    int iterations;
};

int main() {
    SizeClass sizes[] = {
        {"L1D  (~16KB,  half of 32KB/core)", 64, 200'000},
        {"L2   (~256KB, half of 512KB/core)", 256, 5'000},
        {"L3   (~8MB,   half of 16MB shared)", 1448, 20},
        {"DRAM (~256MB, well beyond L3)", 8192, 3},
    };

    for (auto& sc : sizes) {
        std::vector<int> a((size_t)sc.n * sc.n, 1);
        size_t bytes = (size_t)sc.n * sc.n * sizeof(int);

        double row_ms = time_ms([&] { do_not_optimize(traverse_row_major(a, sc.n)); }, sc.iterations);
        double col_ms = time_ms([&] { do_not_optimize(traverse_column_major(a, sc.n)); }, sc.iterations);

        printf("%s -- n=%d, %zu KB\n", sc.label, sc.n, bytes / 1024);
        printf("  row-major:    %.5f ms\n", row_ms);
        printf("  column-major: %.5f ms\n", col_ms);
        printf("  slowdown:     %.2fx\n\n", col_ms / row_ms);
    }

    printf("Expect a real gap even at L1 size (vectorization: row-major\n");
    printf("auto-vectorizes, column-major can't). On top of that, expect\n");
    printf("cache-miss penalties to compound at EVERY level boundary the\n");
    printf("array crosses (L1->L2, L2->L3, L3->DRAM), not just the last\n");
    printf("one -- though in practice the ordering between adjacent\n");
    printf("tiers can be noisy; the L1-vs-DRAM extremes are the reliable\n");
    printf("part of this result.\n");

    return 0;
}