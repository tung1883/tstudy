#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - TODO
 */

/*
 * Problem 70: Climbing Stairs
 * https://leetcode.com/problems/climbing-stairs/
 *
 * Input:
 *   n - number of steps to the top, 1 <= n <= 45
 *
 * Output:
 *   Number of distinct ways to climb to the top, where each move is
 *   either 1 or 2 steps.
 *
 * Examples:
 *   n = 2 -> 2   (1+1, 2)
 *   n = 3 -> 3   (1+1+1, 1+2, 2+1)
 */
int array[43];

int climbStairs(int n) {
    if (n <= 2) return n;

    if (!array[n - 3]) array[n - 3] = climbStairs(n - 1) + climbStairs(n - 2);
    return array[n - 3];
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int n;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = climbStairs(tc.n);
    int pass = got == tc.expected;
    printf("[%s] n=%d -> got=%d, expected=%d -> %s\n", tc.name, tc.n, got, tc.expected, pass ? "PASS" : "FAIL");
}

/* ---------- benchmark ---------- */

static size_t current_working_set_bytes(void) {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    return 0;
#endif
}

/*
 * clock() only has ~1ms resolution on Windows, which is too coarse for a
 * call that itself takes a fraction of a millisecond. QueryPerformanceCounter
 * gives sub-microsecond resolution, so it actually resolves real differences.
 */
static double high_res_ms(void) {
#ifdef _WIN32
    static LARGE_INTEGER frequency = {0};
    if (frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&frequency);
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    return 1000.0 * clock() / CLOCKS_PER_SEC;
#endif
}

static void benchmark(void) {
    int iterations = 100000;
    int n = 45; // max constraint

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = climbStairs(n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (climbStairs call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", 2, 2});
    run_test((TestCase){"example 2", 3, 3});
    run_test((TestCase){"n = 1", 1, 1});
    run_test((TestCase){"n = 4", 4, 5});
    run_test((TestCase){"n = 45 (max)", 45, 1836311903});

    // benchmark();

    return 0;
}
