#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - TODO
 */

/*
 * Problem 8: String to Integer (atoi)
 * https://leetcode.com/problems/string-to-integer-atoi/
 *
 * Input:
 *   s - string, 0 <= s.length <= 200
 *       consists of English letters, digits, ' ', '+', '-', and '.'
 *
 * Output:
 *   The integer that s represents, following these rules:
 *     1. Skip leading whitespace.
 *     2. Optional '+' or '-' sign.
 *     3. Read digits until a non-digit is found.
 *     4. Clamp the result to [-2^31, 2^31 - 1].
 *   Returns 0 if no valid conversion could be performed.
 */
int myAtoi(char* s) {
    int res = 0;
    int sign = 1;
    int i = 0;

    while (s[i] == ' ') i++;
    if (s[i] == '-') { sign = -1; i++; }
    else if (s[i] == '+') i++;
    
    while (s[i] != '\0') {
        if (s[i] < 48 || s[i] > 57) return res;
        if (res > 214748364 || res == 214748364 && s[i] >= 55 ) return 2147483647;
        if (res < -214748364 || res == -214748364 && s[i] >= 56) return -2147483648;

        if (i == 1 && s[i] == '1') {
            printf("%d, %d, %d\n", res, sign, s[i] - 48);
        }
        
        res = res * 10 + sign * (s[i] - 48);
        i++;
    }
    
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    char buf[256];
    strcpy(buf, tc.s);
    int got = myAtoi(buf);
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
    int iterations = 1000000;
    char x[] = "   -2147483648 trailing garbage"; // worst case: leading spaces, sign, max digits, then junk

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    char buf[256];
    for (int k = 0; k < iterations; ++k) {
        strcpy(buf, x);
        result = myAtoi(buf);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] x=\"%s\", result=%d, avg over %d calls, elapsed=%.5f ms\n", x, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (myAtoi call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", "42", 42});
    run_test((TestCase){"example 2", "   -42", -42});
    run_test((TestCase){"example 3", "4193 with words", 4193});
    run_test((TestCase){"no digits", "words and 987", 0});
    run_test((TestCase){"overflow positive", "91283472332", 2147483647});
    run_test((TestCase){"overflow negative", "-91283472332", -2147483648});
    run_test((TestCase){"leading plus", "+1", 1});
    run_test((TestCase){"empty string", "", 0});
    run_test((TestCase){"only whitespace", "   ", 0});
    run_test((TestCase){"leading zeros", "0032", 32});

    benchmark();

    return 0;
}
