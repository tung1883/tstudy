#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Problem 1: Two Sum
 * https://leetcode.com/problems/two-sum/
 *
 * Input:
 *   nums     - array of integers, 2 <= nums.length <= 10^4, -10^9 <= nums[i] <= 10^9
 *   numsSize - length of nums
 *   target   - integer, -10^9 <= target <= 10^9
 *   Exactly one valid answer is guaranteed to exist; may not use the same element twice.
 *
 * Output:
 *   Heap-allocated array of 2 indices [i, j] such that nums[i] + nums[j] == target.
 *   *returnSize is set to 2 (or 0 if no answer, which shouldn't happen per constraints).
 */

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */

typedef struct Node {
    int key;
    int value;
    struct Node* next;
} Node;

typedef struct {
    Node** entries;
    int capacity;
    int size;
} HashTable;

HashTable hashTableCreate(int capacity) {
    HashTable table;
    table.entries = calloc(capacity, sizeof(Node*));
    table.capacity = capacity;
    table.size = 0;
    return table;
}

int hashTableIndex(HashTable* ht, int key) {
    if (key == INT_MIN) key = 0;
    return (key > 0) ? key % ht->capacity : -key % ht->capacity;
}

void insert(HashTable* ht, int key, int value) {
    int index = hashTableIndex(ht, key);
    Node* current = ht->entries[index];

    while (current != NULL) {
        if (current->key == key) {
            current->value = value;
            return; 
        }

        current = current->next;
    }

    Node* newNode = malloc(sizeof(Node));
    newNode->key = key;
    newNode->value = value;
    newNode->next = ht->entries[index];
    ht->entries[index] = newNode;
    ht->size++;
}

/*
 * Looks up `key`. Returns 1 and sets *outValue if found, else returns 0.
 */
int find(HashTable* ht, int key, int* outValue) {
    int index = hashTableIndex(ht, key);
    Node* current = ht->entries[index];

    while (current != NULL) {
        if (current->key == key){
            *outValue = current->value;
            return 1;
        }

        current = current->next;
    }

    return 0;
}

/*
 * Frees every node in every bucket, then the entries array itself.
 * (The HashTable struct itself lives on the stack in twoSum, so it
 * doesn't need freeing - only its heap-allocated contents do.)
 */
void hashTableFree(HashTable* ht) {
    for (int i = 0; i < ht->capacity; i++) {
        Node* old = NULL;
        while (ht->entries[i] != NULL) {
            old = ht->entries[i];
            ht->entries[i] = ht->entries[i]->next;
            free(old);
        }
    }
    
    free(ht->entries);
}

int* twoSum(int* nums, int numsSize, int target, int* returnSize) {
    HashTable ht = hashTableCreate(numsSize * 2);

    int* result = malloc(sizeof(int) * 2);

    for (int i = 0; i < numsSize; i++) {
        int complement = target - nums[i];
        int foundIndex;
        
        if (find(&ht, complement, &foundIndex)) {
            result[0] = foundIndex;
            result[1] = i;
            *returnSize = 2;
            hashTableFree(&ht);
            return result;
        } else {
            insert(&ht, nums[i], i);
        }
    }

    free(result);
    hashTableFree(&ht);
    *returnSize = 0;
    return NULL;
}

/* ---------- test harness ---------- */

typedef struct {
    int* nums;
    int numsSize;
    int target;
    int expected0;
    int expected1;
} TestCase;

static void run_test(const char* name, TestCase tc) {
    int returnSize = 0;
    int* result = twoSum(tc.nums, tc.numsSize, tc.target, &returnSize);

    int pass = result != NULL && returnSize == 2 &&
               ((result[0] == tc.expected0 && result[1] == tc.expected1) ||
                (result[0] == tc.expected1 && result[1] == tc.expected0));

    printf("[%s] target=%d -> ", name, tc.target);
    if (result != NULL && returnSize == 2) {
        printf("got [%d, %d], expected [%d, %d] -> %s\n",
               result[0], result[1], tc.expected0, tc.expected1,
               pass ? "PASS" : "FAIL");
    } else {
        printf("got NULL/empty, expected [%d, %d] -> FAIL\n", tc.expected0, tc.expected1);
    }

    free(result);
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
    int n = 20000;
    int* nums = malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) {
        nums[i] = i * 2; // no two elements sum to an odd target below
    }
    int target = nums[n - 2] + nums[n - 1]; // force worst case: match found at the end

    int iterations = 1000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int returnSize = 0;
    int* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        if (result != NULL) free(result);
        result = twoSum(nums, n, target, &returnSize);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, avg over %d calls, elapsed=%.5f ms\n", n, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (twoSum call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(result);
    free(nums);
}

int main(void) {
    int nums1[] = {2, 7, 11, 15};
    int nums2[] = {3, 2, 4};
    int nums3[] = {3, 3};
    int nums4[] = {-1, -2, -3, -4, -5};
    int nums5[] = {0, 4, 3, 0};

    run_test("example 1", (TestCase){nums1, 4, 9, 0, 1});
    run_test("example 2", (TestCase){nums2, 3, 6, 1, 2});
    run_test("duplicate values", (TestCase){nums3, 2, 6, 0, 1});
    run_test("negative numbers", (TestCase){nums4, 5, -8, 2, 4});
    run_test("zero values", (TestCase){nums5, 4, 0, 0, 3});

    benchmark();

    return 0;
}