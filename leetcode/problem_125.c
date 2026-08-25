#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
 * Problem 125: Valid Palindrome
 * https://leetcode.com/problems/valid-palindrome/
 *
 * Input:
 *   s - string, 1 <= s.length <= 2*10^5
 *       s consists only of printable ASCII characters
 *
 * Output:
 *   true if s is a palindrome after converting all uppercase letters
 *   into lowercase letters and removing all non-alphanumeric characters,
 *   false otherwise.
 */
int isAlphabetic(char c) {
    if (c >= 'a' && c <= 'z' 
        || c >= 'A' && c <= 'Z' 
        || c >= '0' && c <= '9') return 1;
    
    return 0;
}

int isMatched(char c1, char c2) {
    char t1 = c1, t2 = c2;

    if (c1 >= 'A' && c1 <= 'Z') t1 += 'a' - 'A';
    if (c2 >= 'A' && c2 <= 'Z') t2 += 'a' - 'A';
    if (t1 == t2) return 1;
    return 0;
}

int isPalindrome(char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    int i1 = 0, i2 = len - 1;

    while (i1 < i2) {
        if (!isAlphabetic(s[i1])) {
            i1++;
            continue;
        }

        if (!isAlphabetic(s[i2])) {
            i2--;
            continue;
        }

        if (!isMatched(s[i1++], s[i2--])) return 0;
    }
    
    return 1;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    char buf[16384];
    strcpy(buf, tc.s);
    int got = isPalindrome(buf);
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
    int iterations = 1000;
    int n = 200000;
    char* x = malloc(n + 1);
    for (int i = 0; i < n; i++) x[i] = 'a'; // worst case: max length, all matching alnum chars
    x[n] = '\0';

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    char* buf = malloc(n + 1);
    for (int k = 0; k < iterations; ++k) {
        strcpy(buf, x);
        result = isPalindrome(buf);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (isPalindrome call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
    free(x);
    free(buf);
}

int main(void) {
    run_test((TestCase){"example 1", "A man, a plan, a canal: Panama", 1});
    run_test((TestCase){"example 2", "race a car", 0});
    run_test((TestCase){"empty after filtering", " ", 1});
    run_test((TestCase){"single char", "a", 1});
    run_test((TestCase){"mixed case", "0P", 0});
    run_test((TestCase){"numbers", "12321", 1});

    // benchmark();

    return 0;
}
