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
 * Problem 24: Swap Nodes in Pairs
 * https://leetcode.com/problems/swap-nodes-in-pairs/
 *
 * Input:
 *   head - head of a singly-linked list,
 *          0 <= list length <= 100, 0 <= Node.val <= 100
 *
 * Output:
 *   Head of the list with every adjacent pair of nodes swapped.
 *   A trailing odd node stays where it is. The swap must be done by
 *   relinking nodes, not by swapping their values.
 *
 * Examples:
 *   head = [1,2,3,4] -> [2,1,4,3]
 *   head = []        -> []
 *   head = [1]       -> [1]
 *   head = [1,2,3]   -> [2,1,3]   (odd tail carries over untouched)
 */
struct ListNode* swapPairs(struct ListNode* head) {
    struct ListNode dummy = {0, head};
    struct ListNode* ctx = &dummy;

    while (ctx->next != NULL && ctx->next->next != NULL) {
        struct ListNode* first = ctx->next;
        struct ListNode* second = ctx->next->next;

        first->next = second->next;
        second->next = first;
        ctx->next = second;
        ctx = first;
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
    int* vals;
    int size;
    int* expected;
    int expectedSize;
} TestCase;

static void run_test(TestCase tc) {
    struct ListNode* head = build_list(tc.vals, tc.size);

    struct ListNode* result = swapPairs(head);

    int pass = 1;
    struct ListNode* cur = result;
    int i = 0;
    for (; cur != NULL && i < tc.expectedSize; cur = cur->next, i++) {
        if (cur->val != tc.expected[i]) pass = 0;
    }
    if (cur != NULL || i != tc.expectedSize) pass = 0;

    printf("[%s] -> %s\n", tc.name, pass ? "PASS" : "FAIL");

    free_list(result);
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
    int n = 100; /* max list length allowed by the constraints */
    int vals[100];
    for (int i = 0; i < n; i++) {
        vals[i] = i;
    }

    int iterations = 10000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    struct ListNode* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        struct ListNode* head = build_list(vals, n);
        result = swapPairs(head);
        free_list(result);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, avg over %d calls, elapsed=%.5f ms\n", n, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (swapPairs call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    int v1[] = {1, 2, 3, 4};
    int expected1[] = {2, 1, 4, 3};

    int empty[] = {0};

    int v3[] = {1};
    int expected3[] = {1};

    int v4[] = {1, 2, 3};
    int expected4[] = {2, 1, 3};

    int v5[] = {1, 2};
    int expected5[] = {2, 1};

    run_test((TestCase){"example 1", v1, 4, expected1, 4});
    run_test((TestCase){"example 2 - empty", empty, 0, empty, 0});
    run_test((TestCase){"example 3 - single node", v3, 1, expected3, 1});
    run_test((TestCase){"odd length", v4, 3, expected4, 3});
    run_test((TestCase){"exactly one pair", v5, 2, expected5, 2});

    benchmark();

    return 0;
}
