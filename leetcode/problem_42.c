#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * Problem 42: Trapping Rain Water
 * https://leetcode.com/problems/trapping-rain-water/
 *
 * Input:
 *   height     - array of non-negative integers representing an elevation
 *                map where the width of each bar is 1,
 *                1 <= height.length <= 2*10^4
 *                0 <= height[i] <= 10^5
 *   heightSize - length of height
 *
 * Output:
 *   total amount of water that can be trapped after raining.
 */
int trap(int* height, int heightSize) {
    int left = 0, right = heightSize - 1;
    int leftMax = 0, rightMax = 0;
    int water = 0;
    
    while (left < right) {
        if (height[left] < height[right]) {
            if (height[left] >= leftMax) {
                leftMax = height[left];   // new tallest wall so far, no water here
            } else {
                water += leftMax - height[left];  // trapped water at this column
            }
            left++;
        } else {
            if (height[right] >= rightMax) {
                rightMax = height[right];
            } else {
                water += rightMax - height[right];
            }
            right--;
        }
    }

    return water;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* height;
    int heightSize;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = trap(tc.height, tc.heightSize);
    int pass = got == tc.expected;
    printf("[%s] -> got=%d, expected=%d -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int iterations = 1000;
    int n = 20000;
    int* x = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        // worst case: alternating high/low bars, maximizes trapped water
        x[i] = (i % 2 == 0) ? 100000 : 0;
    }

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = trap(x, n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (trap call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
    free(x);
}

int main(void) {
    int t1[] = {0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1};
    int t2[] = {4, 2, 0, 3, 2, 5};
    int t3[] = {1, 1, 1};

    run_test((TestCase){"example 1", t1, 12, 6});
    run_test((TestCase){"example 2", t2, 6, 9});
    run_test((TestCase){"flat, no water", t3, 3, 0});

    // benchmark();

    return 0;
}
