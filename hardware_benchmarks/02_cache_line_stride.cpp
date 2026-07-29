#include <cstdio>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: memory is fetched in whole cache lines (64 bytes = 16
 * ints on most desktop machines), not individual elements. Looping over
 * the SAME large array with different strides, the number of cache
 * lines actually fetched only starts dropping once your stride exceeds
 * one cache line's worth of ints -- below that, every stride still
 * touches every line, so cost stays roughly flat despite doing 2x, 4x,
 * 8x... fewer loop iterations.
 */

static long long touch_stride(const std::vector<int>& a, int stride) {
    long long sum = 0;
    for (size_t i = 0; i < a.size(); i += stride) sum += a[i];
    return sum;
}

int main() {
    const size_t n = 512 * 1024 * 1024 / sizeof(int); // 512MB, doesn't fit in cache
    std::vector<int> a(n, 1);

    for (int stride : {1, 2, 4, 8, 16, 32, 64, 128, 256, 512}) {
        double ms = time_ms([&] { do_not_optimize(touch_stride(a, stride)); }, 3);
        printf("stride %4d ints (%4zu bytes): %.3f ms\n", stride, stride * sizeof(int), ms);
    }

    printf("\nThe talk's idealized version of this is flat up to stride 16\n");
    printf("(one 64-byte cache line) then drops. In practice, on hardware\n");
    printf("with an aggressive prefetcher, you'll more likely see a\n");
    printf("continuous decline -- small strides mean far more loop\n");
    printf("iterations (134M at stride 1 vs 8M at stride 16), so per-\n");
    printf("iteration instruction overhead matters even once the\n");
    printf("prefetcher hides most of the memory latency. The underlying\n");
    printf("point still holds: cost is not proportional to stride, it's\n");
    printf("proportional to distinct cache lines touched.\n");

    return 0;
}
