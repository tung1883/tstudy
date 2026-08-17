#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/* Key notes
- The trick to start i from the end -> 0 is quite neat here
- But sadly this is super duper slow, try to implement the Manacher's alg
*/

/*
 * Problem 5: Longest Palindromic Substring
 * https://leetcode.com/problems/longest-palindromic-substring/
 *
 * Input:
 *   s - string, 1 <= s.length <= 1000, made of digits and English letters
 *
 * Output:
 *   The longest substring of s that is a palindrome. If multiple answers
 *   exist, any one of them is acceptable. Returned as a newly allocated
 *   null-terminated string (caller frees it).
 *
 * Examples:
 *   Input: s = "babad"   Output: "bab" (or "aba")
 *   Input: s = "cbbd"    Output: "bb"
 */

char* longestPalindrome(char* s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }

    // variables to remember the longest palindrome
    int max_len = 1;
    int idx = 0;
    
    bool** table = malloc(len * sizeof(bool*));
    for (int i = 0; i < len; i++) {
        table[i] = malloc(len * sizeof(bool));
    }

    for (int i = len - 1; i >= 0; i--) {
        table[i][i] = true;
        
        for (int j = i + 1; j < len; j++) {
            if (s[i] != s[j]) table[i][j] = false;
            else if (j == i + 1) table[i][j] = true;
            else table[i][j] = table[i + 1][j - 1];

            if (table[i][j] && j - i + 1 > max_len) {
                max_len = j - i + 1;
                idx = i;
            }
        }
    }

    char* result = malloc(max_len * sizeof(char) + 1);
    for (int i = 0; i < max_len; i++) {
        result[i] = s[idx + i]; 
    }
    result[max_len] = '\0';
    return result;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    char* s;
    const char* expected; // one acceptable answer to check length/validity against
} TestCase;

static bool is_palindrome(const char* str) {
    int len = (int)strlen(str);
    for (int i = 0, j = len - 1; i < j; ++i, --j) {
        if (str[i] != str[j]) return false;
    }
    return true;
}

static bool is_substring(const char* needle, const char* haystack) {
    return strstr(haystack, needle) != NULL;
}

static void run_test(TestCase tc) {
    char buf[1024];
    strcpy(buf, tc.s);

    char* got = longestPalindrome(buf);
    bool pass = got != NULL
        && strlen(got) == strlen(tc.expected)
        && is_palindrome(got)
        && is_substring(got, tc.s);

    printf("[%s] got=\"%s\", expected len=%zu -> %s\n",
           tc.name, got ? got : "(null)", strlen(tc.expected), pass ? "PASS" : "FAIL");

    free(got);
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
    int n = 1000;
    char* s = malloc(n + 1);
    for (int i = 0; i < n; ++i) s[i] = 'a' + (i % 3); // repetitive pattern, plenty of palindromes
    s[n] = '\0';

    int iterations = 1000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    char* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        free(result);
        result = longestPalindrome(s);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result_len=%zu, avg over %d calls, elapsed=%.5f ms\n",
           n, result ? strlen(result) : 0, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (longestPalindrome call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(result);
    free(s);
}

int main(void) {
    run_test((TestCase){"example 1", "babad", "bab"});
    run_test((TestCase){"example 2", "cbbd", "bb"});
    run_test((TestCase){"smallest valid input", "a", "a"});
    run_test((TestCase){"all same char", "aaaa", "aaaa"});

    benchmark();

    return 0;
}
