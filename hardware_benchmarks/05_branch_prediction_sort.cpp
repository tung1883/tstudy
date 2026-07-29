#include <algorithm>
#include <cstdio>
#include <random>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: counting how many elements of a vector<float> are > 0
 * runs much slower on unsorted data than sorted data on MSVC, because
 * that compiler emits an actual conditional branch, and a random +-1
 * sequence is the one thing a branch predictor can never predict.
 * GCC/Clang tend to emit a branchless cmov/select sequence instead, so
 * on those compilers you may see little or no difference -- that's the
 * point: same algorithm, same instruction count, wildly different
 * hardware behavior depending on codegen.
 */

static long long count_positive(const std::vector<float>& a) {
    long long count = 0;
    for (float x : a) {
        if (x > 0.0f) ++count;
    }
    return count;
}

int main() {
    const size_t n = 32 * 1024; // small enough to fit in cache -- isolates branch cost
    std::vector<float> a(n);

    std::mt19937 rng(1);
    std::uniform_int_distribution<int> coin(0, 1);
    for (auto& x : a) x = coin(rng) ? 1.0f : -1.0f;

    std::vector<float> sorted_a = a;
    std::sort(sorted_a.begin(), sorted_a.end());

    int iterations = 2000;
    double unsorted_ms = time_ms([&] { do_not_optimize(count_positive(a)); }, iterations);
    double sorted_ms = time_ms([&] { do_not_optimize(count_positive(sorted_a)); }, iterations);

    printf("unsorted: %.5f ms\n", unsorted_ms);
    printf("sorted:   %.5f ms\n", sorted_ms);
    printf("ratio:    %.2fx\n", unsorted_ms / sorted_ms);

    printf("\nOn MSVC expect sorted several times faster (talk saw ~6x).\n");
    printf("On GCC/Clang with optimizations on, expect the ratio near 1x --\n");
    printf("check the generated asm (-S) to see whether it's cmov or a jcc.\n");

    return 0;
}
