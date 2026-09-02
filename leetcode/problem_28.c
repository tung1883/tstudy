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
 * - A naive approach for this problem with big O = len1 * len2
 * - Remember to add Rabin-Karp and KMP
 */

/*
 * Problem 28: Find the Index of the First Occurrence in a String
 * https://leetcode.com/problems/find-the-index-of-the-first-occurrence-in-a-string/
 *
 * Input:
 *   haystack - string to search in, 1 <= haystack.length <= 10^4, lowercase English
 *   needle   - string to search for, 1 <= needle.length <= 10^4, lowercase English
 *
 * Output:
 *   Index of the first occurrence of needle in haystack, or -1 if needle is
 *   not part of haystack.
 *
 * Examples:
 *   haystack = "sadbutsad", needle = "sad"   -> 0   (matches at index 0 and 6, first is 0)
 *   haystack = "leetcode",  needle = "leeto" -> -1  (never occurs)
 */
int stringCmp(char* s1, char* s2, int len) {
    for (int i = 0; i < len; ++i) {
        if (s1[i] != s2[i]) return 0;
    }

    return 1;
}

int strStr(char* haystack, char* needle) {
    int hlen = 0, nlen = 0, i, j;
    while (haystack[hlen] != '\0') ++hlen;
    while (needle[nlen] != '\0') ++nlen;
    if (nlen > hlen) return -1;

    for (i = 0; i + nlen <= hlen; ++i) {
        int res = 1;
        
        for (j = 0; j < nlen; ++j) {
            if (haystack[i + j] != needle[j]) { res = 0; break; }
        }

        if (res) return i;
    }
    
    return -1;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    const char* haystack;
    const char* needle;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    /* strStr takes non-const pointers */
    char hbuf[16384];
    char nbuf[16384];
    strcpy(hbuf, tc.haystack);
    strcpy(nbuf, tc.needle);

    int got = strStr(hbuf, nbuf);
    int pass = got == tc.expected;
    printf("[%s] -> got=%d, expected=%d -> %s\n", tc.name, got, tc.expected, pass ? "PASS" : "FAIL");
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
    /* worst case for naive scan: many near-matches then a mismatch at the end */
    char haystack[10001];
    memset(haystack, 'a', 10000);
    haystack[10000] = '\0';
    char needle[101];
    memset(needle, 'a', 100);
    needle[99] = 'b'; /* forces a full re-scan at every start position */
    needle[100] = '\0';

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = strStr(haystack, needle);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] len(haystack)=%d, len(needle)=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n",
           (int)strlen(haystack), (int)strlen(needle), result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (strStr call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", "sadbutsad", "sad", 0});
    run_test((TestCase){"example 2 - not found", "leetcode", "leeto", -1});
    run_test((TestCase){"needle == haystack", "abc", "abc", 0});
    run_test((TestCase){"match at end", "mississippi", "pi", 9});
    run_test((TestCase){"single char", "a", "a", 0});

    // benchmark();

    return 0;
}
