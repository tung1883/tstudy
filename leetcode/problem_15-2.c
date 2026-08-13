#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - Hashtable solution leads to too much overhead because of all the heap allocaitons,
 * so I re-wrote using 2 pointers
 * - Another example of how big-O can lie to us
 */

/*
 * Problem 15: 3Sum
 * https://leetcode.com/problems/3sum/
 *
 * Input:
 *   nums     - array of integers, 3 <= nums.length <= 3000
 *              -10^5 <= nums[i] <= 10^5
 *
 * Output:
 *   Heap-allocated array of triplets [nums[i], nums[j], nums[k]] (i != j != k)
 *   such that nums[i] + nums[j] + nums[k] == 0, with no duplicate triplets.
 *   *returnSize is set to the number of triplets found.
 *   *returnColumnSizes is a heap-allocated array where each entry is 3
 *   (the length of that triplet), one per returned row.
 */

/**
 * Note: The returned array and *returnColumnSizes must be malloced,
 * assume caller calls free() on both (and on each row).
 */

int** doubleArray(int** arr, int length) {
    int** newArray = malloc(length * 2 * sizeof(int*));
    for (int i = 0; i < length; i++) {
        newArray[i] = arr[i]; 
    }

    free(arr);
    return newArray;
}

void addElement(int*** arrPtr, int *length, int *capacity, int* element) {
    if (*length == *capacity) {
        *arrPtr= doubleArray(*arrPtr, *length);
        *capacity *= 2;
    }
    
    (*arrPtr)[(*length)++] = element;
}

int compareInteger(const void *a, const void *b) {
    return *(const int*) a - *(const int*) b;
}

int** threeSum(int* nums, int numsSize, int* returnSize, int** returnColumnSizes) {
    qsort(nums, numsSize, sizeof(int), compareInteger);

    int capacity = 512, length = 0;
    int** res = malloc(capacity * sizeof(int*));
    int low, high, sum;
    int dataCapacity = capacity * 3;
    
    for (int i = 0; i < numsSize - 2; i++) {
        if (i > 0 && nums[i] == nums[i - 1]) continue;

        low = i + 1, high = numsSize - 1;

        while (low < high) {
            sum = nums[i] + nums[low] + nums[high];

            if (sum == 0) {
                int* triplet = malloc(3 * sizeof(int));
                triplet[0] = nums[i];
                triplet[1] = nums[low];
                triplet[2] = nums[high];
                addElement(&res, &length, &capacity, triplet);
                low++, high--;
                while (low < high && nums[low] == nums[low - 1]) low++;
                while (low < high && nums[high] == nums[high + 1]) high--;
            } else if (sum < 0) {
                low++;
            } else {
                high--;
            }
        }
        
    }

    *returnSize = length;
    int* colSizes = malloc(length * sizeof(int));
    for (int i = 0; i < length; i++) colSizes[i] = 3;
    *returnColumnSizes = colSizes;
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* nums;
    int numsSize;
    int (*expected)[3];
    int expectedCount;
} TestCase;

static int cmp_int(const void* a, const void* b) {
    return (*(const int*)a - *(const int*)b);
}

static int cmp_triplet(const void* a, const void* b) {
    const int* t1 = *(const int**)a;
    const int* t2 = *(const int**)b;
    for (int i = 0; i < 3; i++) {
        if (t1[i] != t2[i]) return t1[i] - t2[i];
    }
    return 0;
}

static void run_test(TestCase tc) {
    int returnSize = 0;
    int* returnColumnSizes = NULL;
    int** result = threeSum(tc.nums, tc.numsSize, &returnSize, &returnColumnSizes);

    int pass = (returnSize == tc.expectedCount);

    if (pass && returnSize > 0) {
        int** sorted_result = malloc(returnSize * sizeof(int*));
        for (int i = 0; i < returnSize; i++) {
            sorted_result[i] = result[i];
            qsort(sorted_result[i], 3, sizeof(int), cmp_int);
        }
        qsort(sorted_result, returnSize, sizeof(int*), cmp_triplet);

        int** sorted_expected = malloc(tc.expectedCount * sizeof(int*));
        for (int i = 0; i < tc.expectedCount; i++) {
            sorted_expected[i] = malloc(3 * sizeof(int));
            memcpy(sorted_expected[i], tc.expected[i], 3 * sizeof(int));
            qsort(sorted_expected[i], 3, sizeof(int), cmp_int);
        }
        qsort(sorted_expected, tc.expectedCount, sizeof(int*), cmp_triplet);

        for (int i = 0; i < returnSize; i++) {
            for (int j = 0; j < 3; j++) {
                if (sorted_result[i][j] != sorted_expected[i][j]) pass = 0;
            }
        }

        for (int i = 0; i < tc.expectedCount; i++) free(sorted_expected[i]);
        free(sorted_expected);
        free(sorted_result);
    }

    printf("[%s] got %d triplet(s), expected %d -> %s\n", tc.name, returnSize, tc.expectedCount, pass ? "PASS" : "FAIL");

    if (result != NULL) {
        for (int i = 0; i < returnSize; i++) free(result[i]);
        free(result);
    }
    free(returnColumnSizes);
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
    int numsSize = 3000;
    int* nums = malloc(numsSize * sizeof(int));
    for (int i = 0; i < numsSize; i++) {
        nums[i] = (i % 2000) - 1000; // dense range around zero, worst case for triplet matching
    }

    int iterations = 20;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int returnSize = 0;
    int* returnColumnSizes = NULL;
    int** result = NULL;
    for (int k = 0; k < iterations; ++k) {
        if (result != NULL) {
            for (int i = 0; i < returnSize; i++) free(result[i]);
            free(result);
            free(returnColumnSizes);
        }
        result = threeSum(nums, numsSize, &returnSize, &returnColumnSizes);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] numsSize=%d, triplets=%d, avg over %d calls, elapsed=%.5f ms\n", numsSize, returnSize, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (threeSum call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    if (result != NULL) {
        for (int i = 0; i < returnSize; i++) free(result[i]);
        free(result);
        free(returnColumnSizes);
    }
    free(nums);
}

int main(void) {
    int nums1[] = {-1, 0, 1, 2, -1, -4};
    int expected1[][3] = {{-1, -1, 2}, {-1, 0, 1}};

    int nums2[] = {0, 1, 1};
    int expected2[][3] = {{0}}; // unused, expectedCount is 0

    int nums3[] = {0, 0, 0};
    int expected3[][3] = {{0, 0, 0}};

    int nums4[] = {-2, 0, 1, 1, 2};
    int expected4[][3] = {{-2, 0, 2}, {-2, 1, 1}};

    int nums5[] = {3, -2, 1, 0};
    int expected5[][3] = {{0}}; // unused, expectedCount is 0

    run_test((TestCase){"example 1", nums1, 6, expected1, 2});
    run_test((TestCase){"no triplets", nums2, 3, expected2, 0});
    run_test((TestCase){"all zeros", nums3, 3, expected3, 1});
    run_test((TestCase){"duplicates in input", nums4, 5, expected4, 2});
    run_test((TestCase){"no zero-sum combo", nums5, 4, expected5, 0});

    benchmark();

    return 0;
}
