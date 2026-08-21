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

struct ListNode {
    int val;
    struct ListNode *next;
};

/*
 * Problem 23: Merge k Sorted Lists
 * https://leetcode.com/problems/merge-k-sorted-lists/
 *
 * Input:
 *   lists     - array of heads of k sorted (ascending) singly-linked lists,
 *               any entry may be NULL (empty list)
 *   listsSize - k, 0 <= k <= 10^4
 *               0 <= lists[i].length <= 500, -10^4 <= Node.val <= 10^4
 *               the total number of nodes across all lists is <= 10^4
 *
 * Output:
 *   Head of one sorted list containing every node of every input list,
 *   or NULL when there are no nodes at all.
 *
 * Examples:
 *   lists = [[1,4,5],[1,3,4],[2,6]] -> [1,1,2,3,4,4,5,6]
 *     (the lists merge into one sorted sequence)
 *   lists = []                      -> []
 *   lists = [[]]                    -> []
 */
struct ListNode* mergeKLists(struct ListNode** lists, int listsSize) {
    int* isNull = malloc(listsSize * sizeof(int));
    for (int i = 0; i < listsSize; i++) isNull[i] = 0;
    int nullCnt = 0;
    int minIdx;
    struct ListNode dummy = { 0, NULL };
    struct ListNode* res = &dummy;
        
    while (nullCnt != listsSize) {
        minIdx = -1;
        
        for (int i = 0; i < listsSize; i++) {
            if (lists[i] == NULL) { 
                if (isNull[i] == 0) {
                    isNull[i] = 1;
                    nullCnt++;
                }

                continue;
            }

            if (minIdx == -1 || lists[i]->val < lists[minIdx]->val) {
                minIdx = i;
            }
           
        }
       
        if (minIdx == -1) break; // safeguard in the final pass
        struct ListNode* temp = lists[minIdx]->next;
        res->next = lists[minIdx];
        res = res->next;
        res->next = NULL;
        lists[minIdx] = temp;
    }

    return dummy.next;
}

/* ---------- test harness ---------- */

static struct ListNode* build_list(int* vals, int n) {
    struct ListNode dummy = {0, NULL};
    struct ListNode* tail = &dummy;
    for (int i = 0; i < n; i++) {
        tail->next = malloc(sizeof(struct ListNode));
        tail->next->val = vals[i];
        tail->next->next = NULL;
        tail = tail->next;
    }
    return dummy.next;
}

static void free_list(struct ListNode* head) {
    while (head != NULL) {
        struct ListNode* next = head->next;
        free(head);
        head = next;
    }
}

typedef struct {
    const char* name;
    int** vals;      /* vals[i] = values of list i */
    int* sizes;      /* sizes[i] = length of list i */
    int listsSize;
    int* expected;
    int expectedSize;
} TestCase;

static void run_test(TestCase tc) {
    struct ListNode** lists = NULL;
    if (tc.listsSize > 0) {
        lists = malloc(sizeof(struct ListNode*) * tc.listsSize);
        for (int i = 0; i < tc.listsSize; i++) {
            lists[i] = build_list(tc.vals[i], tc.sizes[i]);
        }
    }

    struct ListNode* result = mergeKLists(lists, tc.listsSize);

    int pass = 1;
    struct ListNode* cur = result;
    int i = 0;
    for (; cur != NULL && i < tc.expectedSize; cur = cur->next, i++) {
        if (cur->val != tc.expected[i]) pass = 0;
    }
    if (cur != NULL || i != tc.expectedSize) pass = 0;

    printf("[%s] -> %s\n", tc.name, pass ? "PASS" : "FAIL");

    free_list(result);
    free(lists);
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
    int k = 20;
    int n = 500; /* max nodes per list */
    int (*vals)[500] = malloc(sizeof(int) * k * n);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < n; j++) {
            vals[i][j] = j * k + i; /* lists interleave perfectly: worst case */
        }
    }

    int iterations = 200; /* one call = k*n nodes built, merged and freed */
    struct ListNode** lists = malloc(sizeof(struct ListNode*) * k);
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    struct ListNode* result = NULL;
    for (int it = 0; it < iterations; ++it) {
        for (int i = 0; i < k; i++) lists[i] = build_list(vals[i], n);
        result = mergeKLists(lists, k);
        free_list(result);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] k=%d, n=%d each, avg over %d calls, elapsed=%.5f ms\n", k, n, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (mergeKLists call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(lists);
    free(vals);
}

int main(void) {
    int a[] = {1, 4, 5};
    int b[] = {1, 3, 4};
    int c[] = {2, 6};
    int* vals1[] = {a, b, c};
    int sizes1[] = {3, 3, 2};
    int expected1[] = {1, 1, 2, 3, 4, 4, 5, 6};

    int empty[] = {0};
    int* vals3[] = {empty};
    int sizes3[] = {0};

    int single[] = {7};
    int* vals4[] = {single};
    int sizes4[] = {1};

    run_test((TestCase){"example 1", vals1, sizes1, 3, expected1, 8});
    run_test((TestCase){"example 2 - no lists", NULL, NULL, 0, empty, 0});
    run_test((TestCase){"example 3 - one empty list", vals3, sizes3, 1, empty, 0});
    run_test((TestCase){"single node", vals4, sizes4, 1, single, 1});

    benchmark();

    return 0;
}
