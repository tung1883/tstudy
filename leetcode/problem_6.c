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
 * Instead of thinking like where each index will be in (a.k.a. the naive_convert()),
 * Think which index each row has (in order)
 * So instead of a 2D matrix to put each index in, we can allocate characters in one go
 */

/*
 * Problem 6: Zigzag Conversion
 * https://leetcode.com/problems/zigzag-conversion/
 *
 * Input:
 *   s       - string, 1 <= s.length <= 1000, s consists of English letters,
 *             ',' and '.'
 *   numRows - int, 1 <= numRows <= 1000
 *
 * Output:
 *   A newly allocated, null-terminated string containing s written in a
 *   zigzag pattern across numRows rows and then read row by row.
 *   Caller is responsible for freeing the returned buffer.
 */

/*
 * Input: s = "PAYPALISHIRING", numRows = 4
 * Output: "PINALSIGYAHRPI"
 * Explanation:
 *   P     I    N
 *   A   L S  I G
 *   Y A   H R
 *   P     I
 *
 * 0  -> (0, 0) -> 0
 * 1  -> (1, 0) -> 7
 * 2  -> (2, 0)
 * 3  -> (3, 0)
 * 4  -> (2, 1)
 * 5  -> (1, 2)
 * 6  -> (0, 3)
 * 7  -> (1, 3)
 * 8  -> (2, 3)
 * 9  -> (3, 3)
 * 10 -> (2, 4)
 * 11 -> (1, 5)
 * 12 -> (0, 6)
 *
 * a pattern requires 2 * n - 2
 * 1st pattern (p = 0): x % (2 * n - 2) -> y = [0, 1, 2, 3, 4, 5]
 * 0 <= y <= n - 1     ==> (y, p)
 * n <= y <= 2 * n - 3 ==> (2*n-2-y, p+y-n+1)
 * m = number of pattern = (x-y)/(2*n-2)
 * p = (n - 1) * m
 * let's say arr[a][b] -> a=(n-1)*(1+(x-y)/(2*n - 2)), b=n-1
 */

char* naive_convert(char* s, int numRows) {
    int s_size = (int) strlen(s);
    char* res = malloc(s_size + 1);
    res[s_size] = '\0';

    if (numRows == 1 || numRows >= s_size) {
        strcpy(res, s);
        return res;
    }

    int width = (numRows - 1) * (1 + (s_size - s_size % (2 * numRows - 2)) / (2 * numRows - 2));
    char** arr = malloc(sizeof(char*) * numRows);
    for (int i = 0; i < numRows; i++) {
        arr[i] = calloc(width, sizeof(char));
    }

    for (int i = 0; i < s_size; i++) {
        int y = i % (2 * numRows - 2);
        int pattern_no = (i - y) / (2 * numRows - 2);
        int p = (numRows - 1) * pattern_no;

        if (y < numRows) {
            arr[y][p] = s[i];
        } else {
            arr[2 * numRows - 2 - y][p + y - numRows + 1] = s[i];
        }
    }

    int count = 0;
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < width; j++) {
            if (arr[i][j]) res[count++] = arr[i][j];
        }
    }

    for (int i = 0; i < numRows; i++) free(arr[i]);
    free(arr);

    return res;
}

char* convert(char* s, int numRows) {
    int s_size = (int) strlen(s);
    char* res = malloc(s_size + 1);
    if (numRows == 1 || numRows >= s_size) {
        strcpy(res, s);
        return res;
    }

    int cycle = 2 * numRows - 2;
    int idx = 0;
    for (int r = 0; r < numRows; r++) {
        for (int i = r; i < s_size; i += cycle) {
            res[idx++] = s[i];
            // i = k*cycle + r
            // j = k*cycle + cycle - r = i + cycle - 2 * r
            int j = i + cycle - 2 * r;

            if (r != 0 && r != numRows - 1 && j < s_size) {
                res[idx++] = s[j];
            }
        }
    }

    res[s_size] = '\0';
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    char* s;
    int numRows;
    const char* expected;
} TestCase;

static void run_test(TestCase tc) {
    char* got = convert(tc.s, tc.numRows);
    int pass = got != NULL && strcmp(got, tc.expected) == 0;
    printf("[%s] s=\"%s\", numRows=%d -> got=\"%s\", expected=\"%s\" -> %s\n",
           tc.name, tc.s, tc.numRows, got ? got : "(null)", tc.expected, pass ? "PASS" : "FAIL");
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
    int len = 1000;
    char* s = malloc(len + 1);
    for (int i = 0; i < len; ++i) s[i] = 'a' + (i % 26);
    s[len] = '\0';
    int numRows = 17;

    int iterations = 1000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    char* result = NULL;
    for (int k = 0; k < iterations; ++k) {
        free(result);
        result = convert(s, numRows);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] len=%d, numRows=%d, avg over %d calls, elapsed=%.5f ms\n", len, numRows, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (convert call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    free(result);
    free(s);
}

int main(void) {
    run_test((TestCase){"example 1", "PAYPALISHIRING", 3, "PAHNAPLSIIGYIR"});
    run_test((TestCase){"example 2", "PAYPALISHIRING", 4, "PINALSIGYAHRPI"});
    run_test((TestCase){"single row", "AB", 1, "AB"});
    run_test((TestCase){"numRows >= length", "AB", 3, "AB"});
    run_test((TestCase){"single char", "A", 1, "A"});
    run_test((TestCase){"numRows = 2", "ABCD", 2, "ACBD"});
    run_test((TestCase){"numRows = length", "ABCD", 4, "ABCD"});
    run_test((TestCase){"numRows far exceeds length", "PAYPAL", 10, "PAYPAL"});
    run_test((TestCase){"single row, longer string", "HELLOWORLD", 1, "HELLOWORLD"});
    run_test((TestCase){"punctuation", "A,B.C", 2, "ABC,."});
    run_test((TestCase){"full alphabet, numRows = 5", "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 5, "AIQYBHJPRXZCGKOSWDFLNTVEMU"});

    // benchmark();

    return 0;
}
