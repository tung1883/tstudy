#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/* Key notes
- TODO
*/

/*
 * Problem 11: Container With Most Water
 * https://leetcode.com/problems/container-with-most-water/
 *
 * Input:
 *   height     - array of non-negative integers, one per vertical line
 *   heightSize - n == height.length, 2 <= n <= 10^5, 0 <= height[i] <= 10^4
 *
 * Output:
 *   The maximum amount of water a container formed by two of the lines
 *   (and the x-axis) can hold. Area between lines i and j is
 *   min(height[i], height[j]) * (j - i).
 *
 * Examples:
 *   Input: height = [1,8,6,2,5,4,8,3,7]   Output: 49
 *   Input: height = [1,1]                 Output: 1
 */
int maxArea(int* height, int heightSize) {
    int left = 0, right = heightSize - 1;
    int max = 0;
    
    while (left < right) {
        int cur_area = right - left;
        if (height[left] > height[right]) {
            cur_area *= height[right--];
        } else {
            cur_area *= height[left++];
        }
    
        if (max < cur_area) max = cur_area;
    }
    
    return max;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* height;
    int heightSize;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = maxArea(tc.height, tc.heightSize);
    int pass = got == tc.expected;
    printf("[%s] got=%d, expected=%d -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int n = 100000;
    int* height = malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) height[i] = (i * 37) % 10001;

    int iterations = 100;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = maxArea(height, n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (maxArea call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(height);
}

int main(void) {
    int h1[] = {1, 8, 6, 2, 5, 4, 8, 3, 7};
    run_test((TestCase){"example 1", h1, 9, 49});

    int h2[] = {1, 1};
    run_test((TestCase){"example 2", h2, 2, 1});

    int h3[] = {4, 3, 2, 1, 4};
    run_test((TestCase){"tallest lines at the ends", h3, 5, 16});

    int h4[] = {1, 2, 1};
    run_test((TestCase){"peak in the middle", h4, 3, 2});

    benchmark();

    return 0;
}
