#pragma once

#include <chrono>
#include <cstdio>

/*
 * Shared timing helper for all benchmarks in this directory.
 * These are hand-rolled micro-benchmarks (the talk itself warns
 * that micro-benchmarks are dangerous -- optimizers can remove the
 * work you're trying to measure, and confirmation bias is easy).
 * Prefer Google Benchmark for anything you actually rely on; this
 * is just enough to reproduce the talk's numbers locally.
 */

struct ScopedTimerResult {
    double ms;
};

template <typename Fn>
double time_ms(Fn&& fn, int iterations) {
    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();
    for (int i = 0; i < iterations; ++i) {
        fn();
    }
    auto end = clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count() / iterations;
}

// Prevents the optimizer from proving a value is unused and deleting the
// whole loop around it. Same idea as Google Benchmark's DoNotOptimize.
template <typename T>
void do_not_optimize(T const& value) {
    asm volatile("" : : "g"(value) : "memory");
}
