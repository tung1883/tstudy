#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
 * Problem 139: Word Break
 * https://leetcode.com/problems/word-break/
 *
 * Input:
 *   s            - target string, 1 <= s.length <= 300, lowercase English
 *   wordDict     - array of distinct dictionary words
 *   wordDictSize - number of words, 1 <= wordDictSize <= 1000
 *                  1 <= wordDict[i].length <= 20, lowercase English
 *
 * Output:
 *   true if s can be segmented into a space-separated sequence of one or
 *   more dictionary words (each word may be reused any number of times),
 *   false otherwise.
 *
 * Examples:
 *   s = "leetcode",     wordDict = ["leet","code"]          -> true  ("leet code")
 *   s = "applepenapple", wordDict = ["apple","pen"]          -> true  ("apple pen apple")
 *   s = "catsandog",    wordDict = ["cats","dog","sand","and","cat"] -> false
 */

bool compare(char* s1, char* s2, int len) {
    for (int i = 0; i < len; ++i) {
        if (s1[i] != s2[i]) return 0;
    }
    
    return 1;
}

bool wordBreak(char* s, char** wordDict, int wordDictSize) {
    /* TODO
     * approach: dp[i] = can s[0..i) be segmented.
     *   dp[0] = true
     *   dp[i] = OR over words w:  i >= len(w) && dp[i-len(w)] && s[i-len(w)..i) == w
     * O(n * wordDictSize * maxWordLen). n <= 300, dict <= 1000, len <= 20.
     */
    int len = 0;
    while (s[len] != '\0') ++len;
    int* word_len = malloc(sizeof(int) * wordDictSize);
    int i, j;
    for (i = 0; i < wordDictSize; i++) {
        j = 0;
        while (wordDict[i][j] != 0) ++j;
        word_len[i] = j;
    }
    
    bool* dp = malloc(len + 1);
    dp[0] = 1;
    for (i = 1; i <= len; ++i) {
        dp[i] = 0;

        for (j = 0; j < wordDictSize; ++j) {
            dp[i] |= i >= word_len[j] && dp[i - word_len[j]] && compare(s + i - word_len[j], wordDict[j], word_len[j]);

            if (dp[i]) break;
        }
    }

    bool res = dp[len];
    free(word_len);
    free(dp);
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* s;
    const char** wordDict;
    int wordDictSize;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    /* copy inputs: wordBreak takes non-const pointers */
    char sbuf[512];
    strcpy(sbuf, tc.s);

    char** dict = malloc(tc.wordDictSize * sizeof(char*));
    for (int i = 0; i < tc.wordDictSize; ++i) {
        dict[i] = malloc(strlen(tc.wordDict[i]) + 1);
        strcpy(dict[i], tc.wordDict[i]);
    }

    int got = wordBreak(sbuf, dict, tc.wordDictSize);
    int pass = got == tc.expected;
    printf("[%s] -> got=%d, expected=%d -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");

    for (int i = 0; i < tc.wordDictSize; ++i) free(dict[i]);
    free(dict);
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
    int iterations = 10000;
    /* worst case: long s of repeated 'a' ending in 'b', dict of all-'a' prefixes */
    char s[301];
    memset(s, 'a', 300);
    s[299] = 'b';
    s[300] = '\0';

    const char* dict[] = {"a", "aa", "aaa", "aaaa", "aaaaa", "aaaaaa",
                          "aaaaaaa", "aaaaaaaa", "aaaaaaaaa", "aaaaaaaaaa"};
    int dictSize = 10;

    char** d = malloc(dictSize * sizeof(char*));
    for (int i = 0; i < dictSize; ++i) {
        d[i] = malloc(strlen(dict[i]) + 1);
        strcpy(d[i], dict[i]);
    }

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = wordBreak(s, d, dictSize);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] len(s)=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n",
           (int)strlen(s), result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (wordBreak call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    for (int i = 0; i < dictSize; ++i) free(d[i]);
    free(d);
}

int main(void) {
    const char* d1[] = {"leet", "code"};
    const char* d2[] = {"apple", "pen"};
    const char* d3[] = {"cats", "dog", "sand", "and", "cat"};
    const char* d4[] = {"a", "b"};
    const char* d5[] = {"abc", "def"};

    run_test((TestCase){"example 1", "leetcode", d1, 2, 1});
    run_test((TestCase){"example 2", "applepenapple", d2, 2, 1});
    run_test((TestCase){"example 3 - impossible", "catsandog", d3, 5, 0});
    run_test((TestCase){"single char", "a", d4, 2, 1});
    run_test((TestCase){"no match", "abcd", d5, 2, 0});

    // benchmark();

    return 0;
}
