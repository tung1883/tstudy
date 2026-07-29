#include <cstdio>
#include <thread>
#include <vector>
#include "bench_common.hpp"

/*
 * From the talk: cache lines, not individual variables, are the unit of
 * coherence between cores. Four threads each incrementing their OWN,
 * completely independent counter can still be dramatically slower than
 * running them one after another -- if those four counters happen to
 * land on the same 64-byte cache line, every write from one core
 * invalidates the other cores' cached copies of that line, even though
 * no actual data is shared. The talk measured ~12x slower in parallel
 * vs sequential for the unpadded case. Padding each counter out to its
 * own cache line fixes it.
 */

constexpr int kIterations = 50'000'000;
constexpr int kThreads = 4;

// volatile forces every increment to actually hit memory -- without it
// the compiler proves the counter is never read anywhere and deletes
// the entire loop (which is exactly what happened on the first pass:
// both sequential and parallel runs reported ~0ms).
struct Unpadded {
    volatile long counter = 0;
};

struct alignas(64) Padded {
    volatile long counter = 0;
};

static void work(volatile long* counter) {
    for (int i = 0; i < kIterations; ++i) (*counter)++;
}

template <typename Counter>
static double run_parallel(std::vector<Counter>& counters) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back(work, &counters[i].counter);
    }
    for (auto& t : threads) t.join();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

template <typename Counter>
static double run_sequential(std::vector<Counter>& counters) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < kThreads; ++i) work(&counters[i].counter);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
    std::vector<Unpadded> unpadded(kThreads);
    std::vector<Padded> padded(kThreads);

    double unpadded_seq_ms = run_sequential(unpadded);
    double unpadded_par_ms = run_parallel(unpadded);

    double padded_seq_ms = run_sequential(padded);
    double padded_par_ms = run_parallel(padded);

    printf("unpadded (same cache line):\n");
    printf("  sequential: %.1f ms\n", unpadded_seq_ms);
    printf("  parallel:   %.1f ms  (%.2fx vs sequential -- higher is WORSE here)\n",
           unpadded_par_ms, unpadded_par_ms / unpadded_seq_ms);

    printf("\npadded (separate cache lines, alignas(64)):\n");
    printf("  sequential: %.1f ms\n", padded_seq_ms);
    printf("  parallel:   %.1f ms  (%.2fx vs sequential)\n",
           padded_par_ms, padded_par_ms / padded_seq_ms);

    printf("\nExpect unpadded parallel to be SLOWER than unpadded sequential\n");
    printf("(false sharing), and padded parallel to actually benefit from\n");
    printf("running on multiple cores.\n");

    return 0;
}
