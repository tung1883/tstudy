#include <cstdio>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: caches are N-way set-associative, not fully associative.
 * If your access stride is a multiple of (cache_size / ways), every
 * address you touch lands in the SAME set, so only `ways` of them can be
 * cached at once no matter how much free space the cache has overall --
 * they keep evicting each other.
 *
 * This targets L1D specifically (32KB, 8-way, 64-byte lines on this
 * Zen3 chip), not L2/L3, and that's a deliberate choice: L1D's
 * (index bits + offset bits) = 12 bits = exactly one 4KB page, so its
 * set index is fully determined by the low 12 bits of the address --
 * which are guaranteed identical between virtual and physical
 * addresses on any normal (non-huge) page. That means, unlike L2/L3
 * (see the previous version of this file / git history), this conflict
 * is reliably reproducible from a plain std::vector with no huge pages
 * or manual physical alignment required.
 *
 * Conflict period = 32KB / 8 ways = 4KB = 1024 ints. A stride of
 * exactly 1024 ints should be much worse than one just off from it.
 */

static long long touch_n(std::vector<int>& a, size_t stride, long long count) {
    long long sum = 0;
    size_t idx = 0;
    for (long long i = 0; i < count; ++i) {
        sum += a[idx];
        idx = (idx + stride) % a.size();
    }
    return sum;
}

int main() {
    // Big enough that each "set" has far more than 8 candidate lines
    // competing for it (forces real eviction pressure), small enough to
    // stay comfortably within L2/L3 so any slowdown we see is really
    // about L1 set conflicts, not going all the way to DRAM.
    const size_t n = 4 * 1024 * 1024; // 16MB
    std::vector<int> a(n, 1);

    const long long count = 20'000'000;
    const size_t kConflictStride = 1024; // 4KB: L1D conflict period on this chip

    for (size_t stride : {(size_t)768, (size_t)1023, kConflictStride, (size_t)1025,
                          (size_t)1280, (size_t)2047, (size_t)2048, (size_t)2049}) {
        double ms = time_ms([&] { do_not_optimize(touch_n(a, stride, count)); }, 3);
        printf("stride %5zu ints (%3zu KB): %.3f ms%s\n", stride, stride * sizeof(int) / 1024, ms,
               (stride == kConflictStride || stride == 2048) ? "   <- L1 conflict period" : "");
    }

    printf("\nExpect stride 1024 (and its multiple 2048) to stand out\n");
    printf("clearly from neighbors one element off -- that's L1 set\n");
    printf("conflicts, not a smooth function of stride size.\n");

    return 0;
}
