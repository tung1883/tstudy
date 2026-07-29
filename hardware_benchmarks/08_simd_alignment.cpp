#include <cstdio>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: a classic multiply-add mixing loop (dst[i] += src[i] * gain)
 * vectorizes cleanly when both arrays start on a SIMD-register-width
 * boundary (16 bytes for SSE = 4 floats). Offsetting one array by a
 * single element breaks that alignment without making the access
 * "unaligned" in the illegal sense -- it just no longer lines up with
 * the SIMD register width, which costs extra shuffle/blend work.
 * The talk saw ~20% slower on a modern chip, up to ~2.5x on an older one.
 */

const int n = 10'000'000;

static void multiply_add(float* dst, const float* src, float gain, int count) {
    for (int i = 0; i < count; ++i) dst[i] += src[i] * gain;
}

int main() {
    // Over-allocate so we can carve out offset views without going out of bounds.
    std::vector<float> src_buf(n + 4, 1.0f);
    std::vector<float> dst_aligned(n + 4, 0.0f);
    std::vector<float> dst_offset(n + 4, 0.0f);

    float* src_aligned_ptr = src_buf.data();
    float* dst_aligned_ptr = dst_aligned.data();

    float* src_offset_ptr = src_buf.data() + 1; // offset by 1 float = 4 bytes
    float* dst_offset_ptr = dst_offset.data() + 2; // offset by 2 floats = 8 bytes

    double aligned_ms = time_ms([&] { multiply_add(dst_aligned_ptr, src_aligned_ptr, 1.5f, n); }, 5);
    double offset_ms = time_ms([&] { multiply_add(dst_offset_ptr, src_offset_ptr, 1.5f, n); }, 5);

    do_not_optimize(dst_aligned);
    do_not_optimize(dst_offset);

    printf("aligned: %.3f ms\n", aligned_ms);
    printf("offset:  %.3f ms\n", offset_ms);
    printf("ratio:   %.2fx\n", offset_ms / aligned_ms);

    printf("\nDifference will vary a lot by CPU generation -- the talk saw\n");
    printf("~20%% on a recent chip and ~2.5x on an older one. Compile with\n");
    printf("-O3 -march=native to make sure SIMD codegen is actually used.\n");

    return 0;
}
