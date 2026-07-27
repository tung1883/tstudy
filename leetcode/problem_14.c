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
 * Problem 14: Longest Common Prefix
 * https://leetcode.com/problems/longest-common-prefix/
 *
 * Input:
 *   strs     - array of strings, 1 <= strs.length <= 200
 *              0 <= strs[i].length <= 200
 *              strs[i] consists of only lowercase English letters
 *
 * Output:
 *   The longest common prefix string among all strings in strs.
 *   Returns "" if there is no common prefix.
 */
char* longestCommonPrefix(char** strs, int strsSize) {
    if (strsSize == 1) return strs[0];
    int min_len = strlen(strs[0]);
    int i = 0;

    for (i = 0; strs[0][i] != '\0'; i++) {
        for (int j = 1; j < strsSize; j++) {
            if (strs[j][i] != strs[0][i]) {
                char* res = malloc(i + 1);
                memcpy(res, strs[0], i);
                res[i] = '\0';
                return res;
            }
        }
    }

    char* res = malloc(i + 1);
    memcpy(res, strs[0], i);
    res[i] = '\0';
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    char** strs;
    int strsSize;
    const char* expected;
} TestCase;

static void run_test(TestCase tc) {
    char* got = longestCommonPrefix(tc.strs, tc.strsSize);
    int pass = (strcmp(got, tc.expected) == 0);
    printf("[%s] got=\"%s\", expected=\"%s\" -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int strsSize = 200;
    char* strs[200];
    for (int i = 0; i < strsSize; i++) {
        strs[i] = "abcdefghijklmnopqrstuvwxyz"; // worst case: identical strings, full prefix scan
    }

    int iterations = 10000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    char* result = "";
    for (int k = 0; k < iterations; ++k) {
        result = longestCommonPrefix(strs, strsSize);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] strsSize=%d, result=\"%s\", avg over %d calls, elapsed=%.5f ms\n", strsSize, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (longestCommonPrefix call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    char* t1[] = {"flower", "flow", "flight"};
    char* t2[] = {"dog", "racecar", "car"};
    char* t3[] = {"single"};
    char* t4[] = {"", "abc"};
    char* t5[] = {"abc", "abc", "abc"};
    char* t6[] = {"a"};

    run_test((TestCase){"example 1", t1, 3, "fl"});
    run_test((TestCase){"example 2", t2, 3, ""});
    run_test((TestCase){"single string", t3, 1, "single"});
    run_test((TestCase){"empty string in list", t4, 2, ""});
    run_test((TestCase){"all identical", t5, 3, "abc"});
    run_test((TestCase){"single char", t6, 1, "a"});

    // benchmark();

    return 0;
}
