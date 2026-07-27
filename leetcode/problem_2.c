#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - Use dummy head pattern when deal with linked list to avoid edge cases
 * - Cool trick when dealing with addition and carry: carry=sum/10; sum%=10;
 */

/*
 * Problem 2: Add Two Numbers
 * https://leetcode.com/problems/add-two-numbers/
 *
 * Input:
 *   l1, l2 - non-empty singly linked lists, each node holding a single
 *            digit 0-9. Digits are stored in reverse order (the 1's digit
 *            is the head node). Neither list has leading zeros, except the
 *            number 0 itself.
 *
 * Output:
 *   A newly allocated singly linked list, also in reverse-digit order,
 *   representing the sum of the two numbers.
 */

typedef struct ListNode {
    int val;
    struct ListNode* next;
} ListNode;

struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2) {
    int carry = 0;
    struct ListNode dummy = {0, NULL};
    struct ListNode* pt = &dummy;

    while (l1 != NULL || l2 != NULL) {
        int sum = carry;
        if (l1 != NULL) sum += l1->val;
        if (l2 != NULL) sum += l2->val;

        carry = sum / 10;
        sum = sum % 10;

        pt->next = malloc(sizeof(struct ListNode));
        pt->next->val = sum;
        pt->next->next = NULL;
        pt = pt->next;
        if (l1 != NULL) l1 = l1->next;
        if (l2 != NULL) l2 = l2->next;
    }

    if (carry == 1) {
        pt->next = malloc(sizeof(struct ListNode));
        pt->next->val = 1;
        pt->next->next = NULL;
    }

    return dummy.next;
}

/* ---------- list helpers ---------- */

static ListNode* make_list(const int* digits, int n) {
    ListNode dummy = {0, NULL};
    ListNode* tail = &dummy;
    for (int i = 0; i < n; i++) {
        tail->next = malloc(sizeof(ListNode));
        tail->next->val = digits[i];
        tail->next->next = NULL;
        tail = tail->next;
    }
    return dummy.next;
}

static void free_list(ListNode* head) {
    while (head) {
        ListNode* next = head->next;
        free(head);
        head = next;
    }
}

static int lists_equal(ListNode* a, ListNode* b) {
    while (a && b) {
        if (a->val != b->val) return 0;
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}

static void print_list(ListNode* head) {
    printf("[");
    for (ListNode* n = head; n; n = n->next) {
        printf("%d%s", n->val, n->next ? "," : "");
    }
    printf("]");
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* l1;
    int l1_size;
    int* l2;
    int l2_size;
    int* expected;
    int expected_size;
} TestCase;

static void run_test(TestCase tc) {
    ListNode* l1 = make_list(tc.l1, tc.l1_size);
    ListNode* l2 = make_list(tc.l2, tc.l2_size);
    ListNode* expected = make_list(tc.expected, tc.expected_size);

    ListNode* got = addTwoNumbers(l1, l2);
    int pass = lists_equal(got, expected);

    printf("[%s] got=", tc.name);
    print_list(got);
    printf(", expected=");
    print_list(expected);
    printf(" -> %s\n", pass ? "PASS" : "FAIL");

    free_list(l1);
    free_list(l2);
    free_list(expected);
    free_list(got);
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
    int len = 100;
    int* digits1 = malloc(len * sizeof(int));
    int* digits2 = malloc(len * sizeof(int));
    for (int i = 0; i < len; i++) {
        digits1[i] = 9;
        digits2[i] = 9;
    }

    int iterations = 10000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    for (int k = 0; k < iterations; k++) {
        ListNode* l1 = make_list(digits1, len);
        ListNode* l2 = make_list(digits2, len);
        ListNode* result = addTwoNumbers(l1, l2);
        free_list(l1);
        free_list(l2);
        free_list(result);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] len=%d, avg over %d calls, elapsed=%.5f ms\n", len, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (addTwoNumbers call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(digits1);
    free(digits2);
}

int main(void) {
    int l1a[] = {2, 4, 3};
    int l2a[] = {5, 6, 4};
    int expa[] = {7, 0, 8};
    run_test((TestCase){"example 1", l1a, 3, l2a, 3, expa, 3});

    int l1b[] = {0};
    int l2b[] = {0};
    int expb[] = {0};
    run_test((TestCase){"both zero", l1b, 1, l2b, 1, expb, 1});

    int l1c[] = {9, 9, 9, 9, 9, 9, 9};
    int l2c[] = {9, 9, 9, 9};
    int expc[] = {8, 9, 9, 9, 0, 0, 0, 1};
    run_test((TestCase){"carry overflow, different lengths", l1c, 7, l2c, 4, expc, 8});

    int l1d[] = {5};
    int l2d[] = {5};
    int expd[] = {0, 1};
    run_test((TestCase){"single digit carry", l1d, 1, l2d, 1, expd, 2});

    benchmark();

    return 0;
}
