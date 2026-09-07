#include <cstdio>
#include <cstring>
#include <vector>
#include "bench_common.hpp"

/*
 * Scalar (non-SIMD) unaligned access, distinct from 08 which is about
 * SIMD register alignment. Here we just repeatedly read+write a stream
 * of individual ints/floats through pointers at byte offset 0 (aligned)
 * vs. offset 1 (guaranteed unaligned, and guaranteed to straddle a
 * 64-byte cache line every 64 elements). On x86 this doesn't fault --
 * the hardware handles it -- but a load/store that crosses a cache-line
 * boundary needs two cache-line accesses instead of one, stitched
 * together internally.
 */

template <typename T>
static long long touch(const unsigned char* base, size_t count, size_t offset) {
    long long sum = 0;
    for (size_t i = 0; i < count; ++i) {
        T v;
        std::memcpy(&v, base + offset + i * sizeof(T), sizeof(T));
        sum += static_cast<long long>(v);
    }
    return sum;
}

template <typename T>
static void run(const char* label) {
    const size_t n = 64 * 1024 * 1024 / sizeof(T);
    std::vector<unsigned char> buf(n * sizeof(T) + sizeof(T), 1);

    double aligned_ms = time_ms([&] { do_not_optimize(touch<T>(buf.data(), n, 0)); }, 10);
    double unaligned_ms = time_ms([&] { do_not_optimize(touch<T>(buf.data(), n, 1)); }, 10);

    printf("%-6s aligned:   %.3f ms\n", label, aligned_ms);
    printf("%-6s unaligned: %.3f ms  (%.2fx)\n", label, unaligned_ms, unaligned_ms / aligned_ms);
}

int main() {
    run<int>("int");
    run<float>("float");
    return 0;
}
