#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/* 
 * Key notes:
 * - To be fast, we only need to check half of x with its reverse back
 * - For example: 1221 -> we will find the reverse back (half-back is 21, that make the reverse 21),
 * then compare with the top 12
 * - 2 cases: x even -> x == reverse, x odd -> x == reverse / 10 (if we let x <= reverse)
 * - Edge cases: 10, 20, 100, etc. -> this breaks our logic
 */

/*
 * Problem 9: Palindrome Number
 * https://leetcode.com/problems/palindrome-number/
 *
 * Input:
 *   x - integer, -2^31 <= x <= 2^31 - 1
 *
 * Output:
 *   1 if x reads the same forwards and backwards, 0 otherwise.
 *   Negative numbers are never palindromes.
 */
bool isPalindrome(int x) {
    if (x < 0) return 0;
    if (x != 0 && x % 10 == 0) return 0;
    if (x < 9) return 1;

    int reverse = 0;
    while (x > reverse) {
        reverse = reverse * 10 + x % 10;
        x /= 10;
    }

    return x == reverse || x == reverse / 10;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int x;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = isPalindrome(tc.x);
    int pass = got == tc.expected;
    printf("[%s] x=%d -> got=%d, expected=%d -> %s\n", tc.name, tc.x, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int iterations = 1000000;
    int x = 1000000001; // large 10-digit palindrome, worst case for digit-reversal work

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = isPalindrome(x);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] x=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", x, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (isPalindrome call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"positive palindrome", 12921, 1});
    run_test((TestCase){"positive non-palindrome", 12345, 0});
    run_test((TestCase){"negative number", -121, 0});
    run_test((TestCase){"single digit", 7, 1});
    run_test((TestCase){"zero", 0, 1});
    run_test((TestCase){"trailing zero, non-palindrome", 10, 0});
    run_test((TestCase){"even length palindrome", 1221, 1});

    benchmark();

    return 0;
}
