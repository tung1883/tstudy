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
 * Problem 21: Merge Two Sorted Lists
 * https://leetcode.com/problems/merge-two-sorted-lists/
 *
 * Input:
 *   list1, list2 - heads of two sorted (ascending) singly-linked lists,
 *                  0 <= list.length <= 50, -100 <= Node.val <= 100
 *
 * Output:
 *   Head of a single sorted list formed by splicing together the nodes
 *   of list1 and list2 (nodes are reused, not copied).
 */
struct ListNode* mergeTwoLists(struct ListNode* list1, struct ListNode* list2) {
    struct ListNode dummy = { 0, NULL };
    struct ListNode* counter = &dummy;
    
    while (list1 != NULL && list2 != NULL) {
        counter->next = malloc(sizeof(struct ListNode));
        
        if (list1->val < list2->val) {
            counter->next->val = list1->val;
            list1 = list1->next;
        } else {
            counter->next->val = list2->val;
            list2 = list2->next;
        }

        counter = counter->next;
    }

    if (list1 != NULL) counter->next = list1;
    if (list2 != NULL) counter->next = list2;
    
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
    int* vals1;
    int size1;
    int* vals2;
    int size2;
    int* expected;
    int expectedSize;
} TestCase;

static void run_test(TestCase tc) {
    struct ListNode* list1 = build_list(tc.vals1, tc.size1);
    struct ListNode* list2 = build_list(tc.vals2, tc.size2);

    struct ListNode* result = mergeTwoLists(list1, list2);

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
    int n = 50;
    int vals1[50], vals2[50];
    for (int i = 0; i < n; i++) {
        vals1[i] = 2 * i;     // even values
        vals2[i] = 2 * i + 1; // odd values, interleaves worst-case with list1
    }

    int iterations = 10000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    struct ListNode* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        struct ListNode* list1 = build_list(vals1, n);
        struct ListNode* list2 = build_list(vals2, n);
        result = mergeTwoLists(list1, list2);
        free_list(result);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d+%d, avg over %d calls, elapsed=%.5f ms\n", n, n, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (mergeTwoLists call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    int v1[] = {1, 2, 4};
    int v2[] = {1, 3, 4};
    int expected1[] = {1, 1, 2, 3, 4, 4};

    int e[] = {0};
    int expected2[] = {0};

    int v3[] = {0};

    run_test((TestCase){"example 1", v1, 3, v2, 3, expected1, 6});
    run_test((TestCase){"both empty", e, 0, e, 0, e, 0});
    run_test((TestCase){"one empty", e, 0, v3, 1, expected2, 1});
    run_test((TestCase){"other empty", v3, 1, e, 0, expected2, 1});

    benchmark();

    return 0;
}
