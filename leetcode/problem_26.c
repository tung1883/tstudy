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
 * Problem 26: Remove Duplicates from Sorted Array
 * https://leetcode.com/problems/remove-duplicates-from-sorted-array/
 *
 * Input:
 *   nums     - array of integers sorted in non-decreasing order,
 *              1 <= nums.length <= 3*10^4
 *              -100 <= nums[i] <= 100
 *   numsSize - length of nums
 *
 * Output:
 *   Remove the duplicates in-place such that each unique element appears
 *   only once, keeping the relative order. Return k, the number of unique
 *   elements. The first k slots of nums must hold those unique values;
 *   the rest may be anything.
 */
int removeDuplicates(int* nums, int numsSize) {
    int j = 0;

    for (int i = 1; i < numsSize; ++i) {
        if (nums[i] != nums[j]) {
            nums[++j] = nums[i];
        }
    }

    return (numsSize == 0) ? 0 : j + 1;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* nums;
    int numsSize;
    int expectedK;
    int expected[64];
} TestCase;

static void run_test(TestCase tc) {
    int buf[64];
    memcpy(buf, tc.nums, tc.numsSize * sizeof(int));
    int k = removeDuplicates(buf, tc.numsSize);

    int pass = k == tc.expectedK;
    for (int i = 0; pass && i < k; i++) {
        if (buf[i] != tc.expected[i]) pass = 0;
    }

    printf("[%s] k=%d, expectedK=%d, front=[", tc.name, k, tc.expectedK);
    for (int i = 0; i < k; i++) printf("%d%s", buf[i], i + 1 < k ? "," : "");
    printf("] -> %s\n", pass ? "PASS" : "FAIL");
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
    int n = 30000;
    int* x = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) x[i] = i / 3; // sorted with runs of duplicates

    int* buf = malloc(n * sizeof(int));
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        memcpy(buf, x, n * sizeof(int));
        result = removeDuplicates(buf, n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n",
           n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (removeDuplicates call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
    free(x);
    free(buf);
}

int main(void) {
    int t1[] = {1, 1, 2};
    int t2[] = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4};
    int t3[] = {1};
    int t4[] = {2, 2, 2, 2};

    run_test((TestCase){"example 1", t1, 3, 2, {1, 2}});
    run_test((TestCase){"example 2", t2, 10, 5, {0, 1, 2, 3, 4}});
    run_test((TestCase){"single", t3, 1, 1, {1}});
    run_test((TestCase){"all same", t4, 4, 1, {2}});

    // benchmark();

    return 0;
}
