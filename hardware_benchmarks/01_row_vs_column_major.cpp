#include <cstdio>
#include <cstdlib>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: traversing a 2D array in row-major order (the way it's
 * actually laid out in memory) is drastically faster than column-major,
 * because CPUs are good at scanning contiguous memory and bad at jumping.
 *
 * Uses a FLAT std::vector<int> with manual a[i*n+j] indexing, not
 * vector<vector<int>> -- the latter is only contiguous within a row,
 * since each inner vector is its own separate heap allocation, which
 * would confound this with an unrelated "extra pointer indirection"
 * cost. A flat array is the standard, clean way to demonstrate this.
 *
 * Sweeps four sizes chosen to sit inside L1D, L2, L3, and beyond L3
 * into main memory (sizes below are tuned for this machine: AMD Zen3,
 * L1D 32KB/core, L2 512KB/core, L3 16MB shared -- recompute for yours
 * if you're on different hardware).
 *
 * CAVEAT, confirmed via -fopt-info-vec: even the L1-sized case (both
 * traversals hit cache regardless of order) shows a real ~4-5x gap,
 * NOT the ~1x you'd expect from cache effects alone. That's a
 * different, compounding effect: row-major's contiguous access lets
 * GCC auto-vectorize the inner loop (32-byte AVX loads), while
 * column-major's strided access can't ("not suitable for strided
 * load") and stays scalar. So what you're seeing across this sweep is
 * two effects layered together -- a roughly constant vectorization
 * advantage present at every size, PLUS a growing cache-miss penalty
 * that dominates once the array outgrows L3 into DRAM.
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
    printf("auto-vectorizes, column-major can't), which then grows much\n");
    printf("larger once the array outgrows L3 into DRAM (cache misses on\n");
    printf("top of the vectorization gap).\n");

    return 0;
}
