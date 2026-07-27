#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/* Key notes
- TODO
*/

/*
 * Problem 3: Longest Substring Without Repeating Characters
 * https://leetcode.com/problems/longest-substring-without-repeating-characters/
 *
 * Input:
 *   s - string of English letters, digits, symbols and spaces
 *       0 <= s.length <= 5 * 10^4
 *
 * Output:
 *   The length of the longest substring of s without repeating characters.
 */

/*
 * Example: pwwke -> result = wke
 * max_len = 1
 * 2 pointers i, j 
 * i = 0; j = 0; -> arr[s[0]] = 0; arr['p'] = 0;
 * i = 0; j = 1; -> arr[s[1]] = 1; arr['w'] = 1;
 * i = 0; j = 2; -> arr['w'] = 1 > i -> max_len = max(max_len, j - i + 1)
 *                  i = arr['w'] + 1 = 2;
 * i = 2; j = 2 -> arr['w'] = 2
 */

int lengthOfLongestSubstring(char* s) {
    int len_s = strlen(s);
    if (len_s <= 1) return len_s;

    int max_len = 1, visited[128], i = 0, j = 0;
    for (int k = 0; k < 128; k++) visited[k] = -1;

    while (1) {
        if (visited[s[j]] >= i) {
            max_len = (max_len > j - i) ? max_len : j - i;
            i = visited[s[j]] + 1;
            continue;
        }

        visited[s[j]] = j;
        j++;

        if (j == len_s) {
            return (max_len > j - i) ? max_len : j - i;
        }
    }
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    char* s_copy = strdup(tc.s);
    int got = lengthOfLongestSubstring(s_copy);
    int pass = (got == tc.expected);
    printf("[%s] got=%d, expected=%d -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
    free(s_copy);
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
    int len = 50000;
    char* s = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        s[i] = 'a' + (i % 26);
    }
    s[len] = '\0';

    int iterations = 100;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        char* s_copy = strdup(s);
        result = lengthOfLongestSubstring(s_copy);
        free(s_copy);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] len=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", len, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (lengthOfLongestSubstring call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(s);
}

int main(void) {
    run_test((TestCase){"example 1", "abcabcbb", 3});
    run_test((TestCase){"example 2", "bbbbb", 1});
    run_test((TestCase){"example 3", "pwwkew", 3});
    run_test((TestCase){"empty string", "", 0});
    run_test((TestCase){"single char", "a", 1});
    run_test((TestCase){"all unique", "abcdef", 6});
    run_test((TestCase){"repeat at end", "abba", 2});
    run_test((TestCase){"with spaces and symbols", "a b!a", 4});
    run_test((TestCase){"repeat at start", "aab", 2});
    run_test((TestCase){"two chars same", "aa", 1});
    run_test((TestCase){"repeat is first char", "abcda", 4});
    run_test((TestCase){"digits and letters", "a1b2a3", 5});
    run_test((TestCase){"all spaces", "    ", 1});
    run_test((TestCase){"long increasing then repeat", "abcdefgha", 8});

    benchmark();

    return 0;
}
