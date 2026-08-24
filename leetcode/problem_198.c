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
 * Problem 198: House Robber
 * https://leetcode.com/problems/house-robber/
 *
 * Input:
 *   nums     - amount of money stashed in each house, in order
 *   numsSize - number of houses, 1 <= numsSize <= 100
 *              0 <= nums[i] <= 400
 *
 * Output:
 *   Maximum amount you can rob in one night without robbing two
 *   adjacent houses (robbing a house trips its alarm and the alarm
 *   of any adjacent house's system, connecting to the police).
 *
 * Examples:
 *   nums = [1,2,3,1]   -> 4  (rob house 1 and 3: 1 + 3 = 4)
 *   nums = [2,7,9,3,1] -> 12 (rob house 1, 3, 5: 2 + 9 + 1 = 12)
 */
int maxInt(int a, int b) {
    if (a > b) return a;
    return b;
}

int dp(int* nums, int numsSize, int* mem) {
    if (numsSize == 0) return 0;
    if (numsSize == 1) return nums[0];
    if (numsSize == 2) return maxInt(nums[0], nums[1]);
    
    if (mem[numsSize] == -1) {
        mem[numsSize] = maxInt(
            nums[numsSize - 1] + dp(nums, numsSize - 2, mem), 
            nums[numsSize - 2] + dp(nums, numsSize - 3, mem)
        );
    }
    
    return mem[numsSize];
}

int rob(int* nums, int numsSize) {
    int* mem = malloc((numsSize + 1) * sizeof(int));
    for (int i = 0; i < numsSize + 1; ++i) {
        mem[i] = -1;
    }
    
    int res = dp(nums, numsSize, mem);
    free(mem);
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* nums;
    int numsSize;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = rob(tc.nums, tc.numsSize);
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
    int iterations = 100000;
    int n = 100; // max constraint
    int nums[100];
    for (int i = 0; i < n; i++) nums[i] = (i * 37) % 401; // spread of values 0..400

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = rob(nums, n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (rob call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    int a[] = {1, 2, 3, 1};
    int b[] = {2, 7, 9, 3, 1};
    int single[] = {5};
    int two[] = {2, 1};
    int zeros[] = {0, 0, 0, 0};

    run_test((TestCase){"example 1", a, 4, 4});
    run_test((TestCase){"example 2", b, 5, 12});
    run_test((TestCase){"single house", single, 1, 5});
    run_test((TestCase){"two houses", two, 2, 2});
    run_test((TestCase){"all zeros", zeros, 4, 0});

    // benchmark();

    return 0;
}
