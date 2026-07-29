#include <cstdint>
#include <cstdio>
#include <cstring>
#include "bench_common.hpp"

/*
 * From the talk: repeatedly multiplying a float by a number close to (but
 * not exactly) 1 is fast for normal values, 1/0 (inf), 0/0 (NaN), and -0 --
 * but if the value slowly shrinks into denormal range (exponent bits all
 * zero, non-zero fraction), the FPU falls back to a slow microcoded path.
 * The talk measured ~30x slowdown. Denormals show up in real audio code
 * as a signal decays toward (but never quite reaches) zero, e.g. in a
 * feedback loop or reverb tail.
 */

static float multiply_many(float x, float factor, int count) {
    for (int i = 0; i < count; ++i) x *= factor;
    return x;
}

int main() {
    const int iterations = 10'000;
    const int muls_per_iter = 10'000;
    const float factor = 0.9999999f; // barely changes the value each time

    struct Case {
        const char* name;
        float value;
    };

    float inf = 1.0f / 0.0f;
    float nan_val = 0.0f / 0.0f;
    float neg_zero = -0.0f;
    float denormal;
    // Smallest normal float is ~1.18e-38. Pick a subnormal comfortably
    // inside that range (not the smallest possible one) so it takes far
    // more than muls_per_iter steps of `* 0.9999999` to underflow to
    // zero -- otherwise the value flushes to zero after the first
    // multiply and the rest of the loop runs fast on zero, hiding the
    // effect we're trying to measure.
    uint32_t bits = 0x00100000; // ~1.47e-39, subnormal
    std::memcpy(&denormal, &bits, sizeof(bits));

    Case cases[] = {
        {"normal (1.0)", 1.0f},
        {"inf (1/0)", inf},
        {"nan (0/0)", nan_val},
        {"neg zero (-0.0)", neg_zero},
        {"denormal (smallest subnormal)", denormal},
    };

    double baseline_ms = -1.0;
    for (auto& c : cases) {
        double ms = time_ms([&] { do_not_optimize(multiply_many(c.value, factor, muls_per_iter)); }, iterations);
        if (baseline_ms < 0) baseline_ms = ms;
        printf("%-32s %.5f ms  (%.1fx normal)\n", c.name, ms, ms / baseline_ms);
    }

    printf("\nThe talk saw ~30x for the denormal case on its (older) test machine.\n");
    printf("If yours shows little or no penalty, that's a real result too --\n");
    printf("many recent Intel/AMD chips have largely fixed the slow microcode\n");
    printf("path for denormals in scalar FP ops. Don't assume it's fixed\n");
    printf("everywhere though: fix in real code is to enable flush-to-zero /\n");
    printf("denormals-are-zero (_MM_SET_FLUSH_ZERO_MODE /\n");
    printf("_MM_SET_DENORMALS_ZERO_MODE on x86 with <xmmintrin.h>/<pmmintrin.h>).\n");

    return 0;
}
