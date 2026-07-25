#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/* Key notes
- Use the trick return function() to force some desired condition (here is making the first array always the smaller one)
- Think of i as boundary, not as an element in the array, so with length-n array, there will be n+1 boundaries. This handles edge cases better
- Generalize the binary search: we use it when (1) have a sorted array; (2) some condition that can be reached by partioning the array
*/

/*
 * Problem 4: Median of Two Sorted Arrays
 * https://leetcode.com/problems/median-of-two-sorted-arrays/
 *
 * Input:
 *   nums1, nums2         - sorted integer arrays, 0 <= size <= 1000 each
 *                           (at least one of the two arrays is non-empty)
 *   nums1Size, nums2Size - lengths of nums1 and nums2
 *   -10^6 <= nums1[i], nums2[i] <= 10^6
 *
 * Output:
 *   The median of the two sorted arrays combined, as a double.
 *   Required time complexity: O(log(min(m, n))).
 */
double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums2Size < nums1Size) {
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }

    int left = 0;
    int right = nums1Size;

    while (left <= right) {
        int i = (left + right) / 2;
        int j = (nums1Size + nums2Size + 1) / 2 - i;
        int Aleft = (i == 0) ? INT_MIN : nums1[i - 1];
        int Aright = (i == nums1Size) ? INT_MAX : nums1[i];
        int Bleft = (j == 0) ? INT_MIN : nums2[j - 1];
        int Bright = (j == nums2Size) ? INT_MAX : nums2[j];

        if (Aleft <= Bright && Bleft <= Aright) {
            // correct partition found
            if ((nums1Size + nums2Size) % 2 == 0) {
                int leftMax = (Aleft > Bleft) ? Aleft : Bleft;
                int rightMin = (Aright < Bright) ? Aright : Bright;
                return (leftMax + rightMin) / 2.0;
            }

            return  (Aleft > Bleft) ? Aleft : Bleft;
        }

        if (Aleft > Bright) right = i - 1;
        else left = i + 1;
    }

    return 0.0;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* nums1;
    int nums1Size;
    int* nums2;
    int nums2Size;
    double expected;
} TestCase;

static void run_test(TestCase tc) {
    double got = findMedianSortedArrays(tc.nums1, tc.nums1Size, tc.nums2, tc.nums2Size);
    int pass = (got - tc.expected < 1e-5) && (tc.expected - got < 1e-5);
    printf("[%s] got=%.5f, expected=%.5f -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int n1 = 1000, n2 = 1000;
    int* a = malloc(n1 * sizeof(int));
    int* b = malloc(n2 * sizeof(int));
    for (int i = 0; i < n1; ++i) a[i] = i * 2;
    for (int i = 0; i < n2; ++i) b[i] = i * 2 + 1;

    int iterations = 1000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    double result = 0.0;
    for (int k = 0; k < iterations; ++k) {
        result = findMedianSortedArrays(a, n1, b, n2);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n1=%d, n2=%d, median=%.5f, avg over %d calls, elapsed=%.5f ms\n", n1, n2, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (findMedianSortedArrays call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(a);
    free(b);
}

int main(void) {
    int nums1[] = {1, 3};
    int nums2[] = {2};
    run_test((TestCase){"example 1", nums1, 2, nums2, 1, 2.00000});

    int nums3[] = {1, 2};
    int nums4[] = {3, 4};
    run_test((TestCase){"example 2", nums3, 2, nums4, 2, 2.50000});

    int nums5[] = {};
    int nums6[] = {1};
    run_test((TestCase){"one empty array", nums5, 0, nums6, 1, 1.00000});

    int nums7[] = {2};
    int nums8[] = {};
    run_test((TestCase){"other empty array", nums7, 1, nums8, 0, 2.00000});

    int nums9[] = {0, 0};
    int nums10[] = {0, 0};
    run_test((TestCase){"all zeros", nums9, 2, nums10, 2, 0.00000});

    int nums11[] = {1, 2, 3, 4, 5};
    int nums12[] = {6, 7, 8, 9, 10};
    run_test((TestCase){"odd total length", nums11, 5, nums12, 5, 5.50000});

    benchmark();

    return 0;
}
