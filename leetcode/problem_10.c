#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - Star with a normal character (not *), look ahead to see if the next one is *
 * - This will make it easier to handle than you checking if the current one is *
 */

/*
 * Problem 10: Regular Expression Matching
 * https://leetcode.com/problems/regular-expression-matching/
 *
 * Input:
 *   s - string, 1 <= s.length <= 20, contains only lowercase English letters
 *   p - pattern, 1 <= p.length <= 20, contains lowercase English letters,
 *       '.', and '*'
 *
 * Output:
 *   true if p matches the entire string s, false otherwise.
 *   '.' matches any single character.
 *   '*' matches zero or more of the preceding element.
 */
bool handle(char* s, char* p, int s_idx, int p_idx, char (*memo)[21]) {
    if (memo[s_idx][p_idx] != -1) return memo[s_idx][p_idx];

    if (p[p_idx] == '\0') memo[s_idx][p_idx] = s[s_idx] == '\0';
    else {
        bool firstMatch = s[s_idx] != '\0' && (p[p_idx] == '.' || s[s_idx] == p[p_idx]);
        
        if (p[p_idx + 1] != '\0' && p[p_idx + 1] == '*') {
            memo[s_idx][p_idx] = handle(s, p, s_idx, p_idx + 2, memo) ||
                     (firstMatch && handle(s, p, s_idx + 1, p_idx, memo));
        } else {
            memo[s_idx][p_idx] = firstMatch && handle(s, p, s_idx + 1, p_idx + 1, memo);
        }
    }

    return memo[s_idx][p_idx];
}

bool isMatch(char* s, char* p) {
    char memo[21][21];
    memset(memo, -1, 441);
    return handle(s, p, 0, 0, memo);
}

// loop version
bool loop_isMatch(char* s, char* p) {
    int slen = strlen(s), plen = strlen(p);
    char dp[21][21];

    dp[slen][plen] = true;                      // both exhausted -> match
    for (int p_idx = plen; p_idx >= 0; p_idx--) {
        for (int s_idx = slen; s_idx >= 0; s_idx--) {
            if (p_idx == plen) {
                if (s_idx == slen) continue;     // already seeded above
                dp[s_idx][p_idx] = false;        // pattern exhausted, string isn't
                continue;
            }

            bool firstMatch = s_idx < slen && (p[p_idx] == '.' || s[s_idx] == p[p_idx]);

            if (p_idx + 1 < plen && p[p_idx + 1] == '*') {
                dp[s_idx][p_idx] = dp[s_idx][p_idx + 2] ||
                                    (firstMatch && dp[s_idx + 1][p_idx]);
            } else {
                dp[s_idx][p_idx] = firstMatch && dp[s_idx + 1][p_idx + 1];
            }
        }
    }

    return dp[0][0];
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    const char* p;
    bool expected;
} TestCase;

static void run_test(TestCase tc) {
    char s_buf[64];
    char p_buf[64];
    strcpy(s_buf, tc.s);
    strcpy(p_buf, tc.p);
    bool got = isMatch(s_buf, p_buf);
    int pass = got == tc.expected;
    printf("[%s] s=\"%s\", p=\"%s\" -> got=%d, expected=%d -> %s\n", tc.name, tc.s, tc.p, got, tc.expected, pass ? "PASS" : "FAIL");
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
    char s[] = "aaaaaaaaaaaaaaaaaaaa"; // 20 chars, worst case for backtracking
    char p[] = "a*a*a*a*a*a*a*a*a*a*";

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    bool result = false;
    char s_buf[64];
    char p_buf[64];
    for (int k = 0; k < iterations; ++k) {
        strcpy(s_buf, s);
        strcpy(p_buf, p);
        result = isMatch(s_buf, p_buf);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] s=\"%s\", p=\"%s\", result=%d, avg over %d calls, elapsed=%.5f ms\n", s, p, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (isMatch call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", "aa", "a", false});
    run_test((TestCase){"example 2", "aa", "a*", true});
    run_test((TestCase){"example 3", "ab", ".*", true});
    run_test((TestCase){"mismatch", "mississippi", "mis*is*p*.", false});
    run_test((TestCase){"mismatch", "mississippi", "mis*is*ip*.", true});
    run_test((TestCase){"empty string and pattern", "", "", true});
    run_test((TestCase){"empty string, star pattern", "", "a*", true});
    run_test((TestCase){"single char match", "c", "c", true});

    benchmark();

    return 0;
}
