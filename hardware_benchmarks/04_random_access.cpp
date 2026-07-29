#include <algorithm>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: sequential access, fixed-stride ("wrong order") access,
 * and fully random access all cause cache misses on a large array, but
 * they are NOT equally bad. Fixed-stride access is helped a lot by the
 * hardware prefetcher, which detects a constant stride and starts
 * fetching ahead of time -- even though the access pattern is "bad".
 * Truly random access defeats the prefetcher entirely and the talk
 * measured it at 100x+ slower than sequential, matching cache latency.
 */

static long long sum_indices(const std::vector<int>& a, const std::vector<size_t>& idx) {
    long long sum = 0;
    for (size_t i : idx) sum += a[i];
    return sum;
}

int main() {
    const size_t n = 64 * 1024 * 1024 / sizeof(int); // 256MB, doesn't fit in cache
    std::vector<int> a(n, 1);

    std::vector<size_t> sequential(n);
    std::iota(sequential.begin(), sequential.end(), 0);

    std::vector<size_t> strided;
    for (size_t i = 0; i < n; i += 16) strided.push_back(i);

    std::vector<size_t> random_idx = sequential;
    std::mt19937 rng(42);
    std::shuffle(random_idx.begin(), random_idx.end(), rng);

    double seq_ms = time_ms([&] { do_not_optimize(sum_indices(a, sequential)); }, 3);
    double strided_ms = time_ms([&] { do_not_optimize(sum_indices(a, strided)); }, 3);
    double random_ms = time_ms([&] { do_not_optimize(sum_indices(a, random_idx)); }, 3);

    // Normalize to a per-1M-element cost so the three are comparable
    // despite touching different numbers of elements.
    double seq_per_m = seq_ms / (sequential.size() / 1e6);
    double strided_per_m = strided_ms / (strided.size() / 1e6);
    double random_per_m = random_ms / (random_idx.size() / 1e6);

    printf("sequential: %.4f ms / 1M elements\n", seq_per_m);
    printf("strided16:  %.4f ms / 1M elements  (%.1fx sequential)\n", strided_per_m, strided_per_m / seq_per_m);
    printf("random:     %.4f ms / 1M elements  (%.1fx sequential)\n", random_per_m, random_per_m / seq_per_m);

    printf("\nNote sequential's edge over strided16 here is mostly about\n");
    printf("reuse -- one 64-byte fetch serves 16 consecutive sequential\n");
    printf("touches but only 1 strided touch, regardless of any\n");
    printf("prefetcher. The prefetcher's contribution specifically shows\n");
    printf("up comparing strided16 to random: both touch a fresh cache\n");
    printf("line every time, but the prefetcher can anticipate the\n");
    printf("constant stride and hide some of that latency, while a fully\n");
    printf("random pattern gets no help at all -- so expect strided16 <\n");
    printf("random, even though neither reuses a line like sequential does.\n");

    return 0;
}
