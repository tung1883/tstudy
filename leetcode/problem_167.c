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
 * Problem 167: Two Sum II - Input Array Is Sorted
 * https://leetcode.com/problems/two-sum-ii-input-array-is-sorted/
 *
 * Input:
 *   numbers - 1-indexed array of integers sorted in non-decreasing order,
 *             2 <= numbers.length <= 3*10^4
 *             -1000 <= numbers[i] <= 1000
 *   target  - integer, -1000 <= target <= 1000
 *
 * Output:
 *   two indices (1-indexed), index1 < index2, such that
 *   numbers[index1] + numbers[index2] == target.
 *   Exactly one solution is guaranteed to exist.
 *   returnSize must be set to 2.
 */
int* twoSum(int* numbers, int numbersSize, int target, int* returnSize) {    
    *returnSize = 2;
    int idx1 = 0, idx2 = numbersSize - 1;

    while (idx1 < idx2) {
        int temp = target - numbers[idx1] - numbers[idx2];

        if (temp == 0) {
            int* res = malloc(sizeof(int) * 2);
            res[0] = idx1 + 1;
            res[1] = idx2 + 1;
            return res;
        }

        if (temp > 0) idx1++;
        else idx2--;
    }
    
    return NULL;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* numbers;
    int numbersSize;
    int target;
    int expected[2];
} TestCase;

static void run_test(TestCase tc) {
    int returnSize = 0;
    int* got = twoSum(tc.numbers, tc.numbersSize, tc.target, &returnSize);
    int pass = got != NULL && returnSize == 2 &&
               got[0] == tc.expected[0] && got[1] == tc.expected[1];
    printf("[%s] target=%d -> got=[%d,%d], expected=[%d,%d] -> %s\n",
           tc.name, tc.target,
           got ? got[0] : -1, got ? got[1] : -1,
           tc.expected[0], tc.expected[1],
           pass ? "PASS" : "FAIL");
    free(got);
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
    for (int i = 0; i < n; i++) x[i] = i - n / 2; // worst case: match found at the very end
    int target = x[n - 2] + x[n - 1];

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int returnSize = 0;
    int* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        free(result);
        result = twoSum(x, n, target, &returnSize);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=[%d,%d], avg over %d calls, elapsed=%.5f ms\n",
           n, result ? result[0] : -1, result ? result[1] : -1, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (twoSum call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
    free(x);
    free(result);
}

int main(void) {
    int t1[] = {2, 7, 11, 15};
    int t2[] = {2, 3, 4};
    int t3[] = {-1, 0};

    run_test((TestCase){"example 1", t1, 4, 9, {1, 2}});
    run_test((TestCase){"example 2", t2, 3, 6, {1, 3}});
    run_test((TestCase){"example 3", t3, 2, -1, {1, 2}});

    // benchmark();

    return 0;
}
