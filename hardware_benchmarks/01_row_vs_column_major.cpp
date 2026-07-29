#include <cstdio>
#include <cstdlib>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: traversing a 2D array in row-major order (the way it's
 * actually laid out in memory) is drastically faster than column-major,
 * because CPUs are good at scanning contiguous memory and bad at jumping.
 * Expect ~30-40x slowdown for the wrong-order traversal once the array
 * no longer fits comfortably in cache.
 */

static long long traverse_row_major(std::vector<std::vector<int>>& a) {
    long long sum = 0;
    for (size_t i = 0; i < a.size(); ++i)
        for (size_t j = 0; j < a[i].size(); ++j)
            sum += a[i][j];
    return sum;
}

static long long traverse_column_major(std::vector<std::vector<int>>& a) {
    long long sum = 0;
    for (size_t j = 0; j < a[0].size(); ++j)
        for (size_t i = 0; i < a.size(); ++i)
            sum += a[i][j];
    return sum;
}

int main() {
    const int n = 4096; // n*n ints ~ 64MB, well beyond L2/L3
    std::vector<std::vector<int>> a(n, std::vector<int>(n, 1));

    int iterations = 5;
    double row_ms = time_ms([&] { do_not_optimize(traverse_row_major(a)); }, iterations);
    double col_ms = time_ms([&] { do_not_optimize(traverse_column_major(a)); }, iterations);

    printf("row-major:    %.3f ms\n", row_ms);
    printf("column-major: %.3f ms\n", col_ms);
    printf("slowdown:     %.1fx\n", col_ms / row_ms);

    return 0;
}
