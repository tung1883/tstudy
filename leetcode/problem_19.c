#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - Implement 2-pointer solution for this
 */

struct ListNode {
    int val;
    struct ListNode *next;
};

/*
 * Problem 19: Remove Nth Node From End of List
 * https://leetcode.com/problems/remove-nth-node-from-end-of-list/
 *
 * Input:
 *   head - head of a singly-linked list, sz == list length, 1 <= sz <= 30
 *          0 <= Node.val <= 100
 *   n    - 1 <= n <= sz, the 1-indexed position from the end to remove
 *
 * Output:
 *   Head of the list with the nth node from the end removed.
 */

struct ListNode* removeNthFromEnd(struct ListNode* head, int n) {
    struct ListNode* dummy = malloc(sizeof(struct ListNode));
    dummy->next = head;
    struct ListNode* counter = dummy;
    int length = 0;
    
    while (counter->next != NULL) {
        length++;
        counter = counter->next;
    }

    int posFromStart = length - n;
    counter = dummy;

    while (posFromStart > 0) {
        posFromStart--;
        counter = counter->next;
    }

    struct ListNode* temp = counter->next;
    counter->next = counter->next->next;
    free(temp);
    temp = dummy->next;
    free(dummy);
    return temp;
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
    int n;
    int* expected;
    int expectedSize;
} TestCase;

static void run_test(TestCase tc) {
    struct ListNode* head = build_list(tc.vals, tc.size);

    struct ListNode* result = removeNthFromEnd(head, tc.n);

    int pass = 1;
    struct ListNode* cur = result;
    int i = 0;
    for (; cur != NULL && i < tc.expectedSize; cur = cur->next, i++) {
        if (cur->val != tc.expected[i]) pass = 0;
    }
    if (cur != NULL || i != tc.expectedSize) pass = 0;

    printf("[%s] n=%d -> %s\n", tc.name, tc.n, pass ? "PASS" : "FAIL");

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
    int n = 30;
    int vals[30];
    for (int i = 0; i < n; i++) {
        vals[i] = i;
    }

    int iterations = 100000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    struct ListNode* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        struct ListNode* head = build_list(vals, n);
        result = removeNthFromEnd(head, 1); // worst case: last node, requires full traversal
        free_list(result);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, avg over %d calls, elapsed=%.5f ms\n", n, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (removeNthFromEnd call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    int v1[] = {1, 2, 3, 4, 5};
    int expected1[] = {1, 2, 3, 5};

    int v2[] = {1};

    int v3[] = {1, 2};
    int expected3[] = {1};

    run_test((TestCase){"example 1", v1, 5, 2, expected1, 4});
    run_test((TestCase){"single node", v2, 1, 1, NULL, 0});
    run_test((TestCase){"remove second of two", v3, 2, 1, expected3, 1});

    benchmark();

    return 0;
}
