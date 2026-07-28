#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - TODO
 */

/*
 * Problem 20: Valid Parentheses
 * https://leetcode.com/problems/valid-parentheses/
 *
 * Input:
 *   s - string, 1 <= s.length <= 10^4
 *       consists of only '(', ')', '{', '}', '[', ']'
 *
 * Output:
 *   true if every open bracket is closed by the same type of bracket
 *   and in the correct order, false otherwise.
 */
bool isValid(char* s) {
    char arr[10000];
    int top = -1;
    int i = 0;

    while (s[i] != '\0') {
        if (s[i] == '{' || s[i] == '[' || s[i] == '(') {
            top++;
            arr[top] = s[i];
            i++;
            continue;
        }

        if (top == -1) return false;
        if (s[i] == '}' && arr[top] != '{') return false;
        if (s[i] == ']' && arr[top] != '[') return false;
        if (s[i] == ')' && arr[top] != '(') return false;

        top--;
        i++;
    }

    return top == -1;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    bool expected;
} TestCase;

static void run_test(TestCase tc) {
    char buf[16384];
    strcpy(buf, tc.s);
    bool got = isValid(buf);
    int pass = got == tc.expected;
    printf("[%s] s=\"%s\" -> got=%d, expected=%d -> %s\n", tc.name, tc.s, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int n = 10000;
    char x[10001];
    for (int i = 0; i < n / 2; i++) x[i] = '(';
    for (int i = n / 2; i < n; i++) x[i] = ')'; // worst case: max length, fully nested/balanced
    x[n] = '\0';

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    bool result = false;
    char buf[10001];
    for (int k = 0; k < iterations; ++k) {
        strcpy(buf, x);
        result = isValid(buf);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (isValid call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", "()", true});
    run_test((TestCase){"example 2", "()[]{}", true});
    run_test((TestCase){"example 3", "(]", false});
    run_test((TestCase){"mismatched order", "([)]", false});
    run_test((TestCase){"nested valid", "{[]}", true});
    run_test((TestCase){"single open", "(", false});
    run_test((TestCase){"single close", ")", false});
    run_test((TestCase){"empty string", "", true});

    // benchmark();

    return 0;
}
