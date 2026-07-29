#include <cstdio>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: a serial dependency chain -- each result depending on
 * the one right before it -- limits you to the CPU's dependency
 * *latency* (e.g. ~3-4 cycles per float add on most chips), even though
 * the CPU could *issue* multiple independent adds per cycle if you gave
 * it something not chained together. The talk's own example of this is
 * an atoi-style accumulator ("sum = sum*10 + digit"), credited there to
 * Andrei Alexandrescu's "Writing Fast Code" talk (~3x speedup by
 * breaking the chain).
 *
 * NOTE: an earlier version of this file tried to reproduce a different
 * slide from the talk (a two-array a[i]/b[i] recurrence "fixed" by
 * shifting the loop body). That version was a genuine mathematical
 * recurrence -- b[i] truly depends on b[i-1] -- which no amount of
 * loop restructuring changes; no compiler can vectorize a real serial
 * recurrence that way, and neither GCC nor Clang did. This file
 * demonstrates the same underlying idea (breaking a dependency chain)
 * with a case that's actually fixable: splitting one long chain into
 * several independent ones.
 */

const int n = 100'000'000;

// One accumulator: every add must wait for the previous add to finish.
// Chain length = n, so total time is bounded by n * (add latency).
static float sum_single_chain(const std::vector<float>& a) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) sum += a[i];
    return sum;
}

// Four independent accumulators: four chains of length n/4 that can
// overlap in the CPU's pipeline (bounded by add *throughput*, not
// latency), combined into one value only at the very end.
static float sum_four_chains(const std::vector<float>& a) {
    float sum0 = 0.0f, sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    int i = 0;
    for (; i + 4 <= n; i += 4) {
        sum0 += a[i];
        sum1 += a[i + 1];
        sum2 += a[i + 2];
        sum3 += a[i + 3];
    }
    float sum = sum0 + sum1 + sum2 + sum3;
    for (; i < n; ++i) sum += a[i];
    return sum;
}

int main() {
    std::vector<float> a(n, 1.0f);

    double single_ms = time_ms([&] { do_not_optimize(sum_single_chain(a)); }, 5);
    double four_ms = time_ms([&] { do_not_optimize(sum_four_chains(a)); }, 5);

    printf("single chain (1 accumulator): %.3f ms\n", single_ms);
    printf("four chains (4 accumulators): %.3f ms\n", four_ms);
    printf("speedup:                      %.2fx\n", single_ms / four_ms);

    printf("\nSame total additions either way -- verified via objdump that\n");
    printf("neither version gets auto-vectorized (zero vaddps/packed SIMD\n");
    printf("instructions in either, only scalar vaddss), so the speedup is\n");
    printf("purely from breaking one long dependency chain into four\n");
    printf("independent ones the CPU's out-of-order execution can overlap.\n");

    return 0;
}
