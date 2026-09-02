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
 * Problem 27: Remove Element
 * https://leetcode.com/problems/remove-element/
 *
 * Input:
 *   nums     - array of integers, 0 <= nums.length <= 100
 *              0 <= nums[i] <= 50
 *   numsSize - length of nums
 *   val      - value to remove, 0 <= val <= 100
 *
 * Output:
 *   Remove all occurrences of val in nums in-place. Return k, the number of
 *   elements not equal to val. The first k slots of nums must hold those
 *   elements (order does not matter); the rest may be anything.
 *
 * Examples:
 *   nums = [3,2,2,3],          val = 3 -> 2, nums[0..2) = [2,2]
 *   nums = [0,1,2,2,3,0,4,2],  val = 2 -> 5, nums[0..5) = [0,1,4,0,3] (any order)
 */
int removeElement(int* nums, int numsSize, int val) {
    int l = 0, r = numsSize - 1;
    int k = 0;

    while (l <= r) {
        if (nums[r] == val) { --r; continue; }
        if (nums[l] != val) { ++l; ++k; continue; }

        nums[l] = nums[r];
        ++l; --r; ++k;
    }
    
    return k;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* nums;
    int numsSize;
    int val;
    int expectedK;
    int expected[64]; /* expected multiset of the first k elements, order-insensitive */
} TestCase;

static void run_test(TestCase tc) {
    int buf[64];
    memcpy(buf, tc.nums, tc.numsSize * sizeof(int));
    int k = removeElement(buf, tc.numsSize, tc.val);

    int pass = k == tc.expectedK;

    /* order does not matter: compare as multisets over the value range [0,50] */
    if (pass) {
        int got[51] = {0}, want[51] = {0};
        for (int i = 0; i < k; i++) {
            if (buf[i] == tc.val) pass = 0;      /* val must be gone from the front */
            got[buf[i]]++;
        }
        for (int i = 0; i < tc.expectedK; i++) want[tc.expected[i]]++;
        for (int i = 0; pass && i < 51; i++) {
            if (got[i] != want[i]) pass = 0;
        }
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
    for (int i = 0; i < n; i++) x[i] = i % 4; /* ~1/4 of the array equals val */

    int* buf = malloc(n * sizeof(int));
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        memcpy(buf, x, n * sizeof(int));
        result = removeElement(buf, n, 2);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n",
           n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (removeElement call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
    free(x);
    free(buf);
}

int main(void) {
    int t1[] = {3, 2, 2, 3};
    int t2[] = {0, 1, 2, 2, 3, 0, 4, 2};
    int t3[] = {1};
    int t4[] = {2, 2, 2, 2};

    run_test((TestCase){"example 1", t1, 4, 3, 2, {2, 2}});
    run_test((TestCase){"example 2", t2, 8, 2, 5, {0, 1, 3, 0, 4}});
    run_test((TestCase){"val absent", t3, 1, 5, 1, {1}});
    run_test((TestCase){"all removed", t4, 4, 2, 0, {}});

    // benchmark();

    return 0;
}
