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
 * Problem 58: Length of Last Word
 * https://leetcode.com/problems/length-of-last-word/
 *
 * Input:
 *   s - string consisting of words and spaces, 1 <= s.length <= 10^4
 *       s contains only English letters and spaces ' '
 *       there is at least one word in s
 *
 * Output:
 *   length of the last word in the string, where a word is a maximal
 *   substring consisting of non-space characters only.
 */
int lengthOfLastWord(char* s) {
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }

    i--;
    while (s[i] == ' ') i--;
    int count = 0;
    while (i - count >= 0 && s[i - count] != ' ') {
        count++;
    }
    
    return count;
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
    int got = lengthOfLastWord(buf);
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
    for (int i = 0; i < n - 1; i++) x[i] = ' '; // worst case: max length, trailing spaces before last word
    x[n - 1] = 'a';
    x[n] = '\0';

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    char buf[10001];
    for (int k = 0; k < iterations; ++k) {
        strcpy(buf, x);
        result = lengthOfLastWord(buf);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (lengthOfLastWord call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", "Hello World", 5});
    run_test((TestCase){"trailing spaces", "   fly me   to   the moon  ", 4});
    run_test((TestCase){"single word", "luffy is still joyboy", 6});
    run_test((TestCase){"one letter", "a", 1});
    run_test((TestCase){"leading and trailing", "  a  ", 1});

    // benchmark();

    return 0;
}
