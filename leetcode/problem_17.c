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
 * Problem 17: Letter Combinations of a Phone Number
 * https://leetcode.com/problems/letter-combinations-of-a-phone-number/
 *
 * Input:
 *   digits - string of digits '2'-'9', 0 <= digits.length <= 4
 *            (standard phone keypad letter mapping applies)
 *
 * Output:
 *   Heap-allocated array of all possible letter combinations the number
 *   could represent, in any order. *returnSize is set to the number of
 *   combinations found. If digits is empty, returns an empty array
 *   (*returnSize = 0).
 */


int m_pow(int base, int exp) {
    int res = 1;
    for (int i = 0; i < exp; i++) res *= base;
    return res;
}

char** letterCombinations(char* digits, int* returnSize) {
    char table[8][4] = { "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
    if (digits[0] == '\0') { *returnSize = 0; return NULL; }

    int len = 0;
    int number_of_79 = 0;

    while (digits[len] != '\0') {
        if (digits[len] == '7' || digits[len] == '9') number_of_79++;
        len++;
    }

    int size = m_pow(4, number_of_79) * m_pow(3, len - number_of_79);
    *returnSize = size;
    char** res = malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        res[i] = malloc(len * sizeof(char) + 1);
        res[i][len] = '\0';
    }

    int blockSize = size;
    
    for (int i = 0; digits[i] != '\0'; i++) {
        int letters = (digits[i] == '7' || digits[i] == '9') ? 4 : 3;
        blockSize /= letters;
        for (int j = 0; j < size; j++) {
            int letterIndex = (j / blockSize) % letters;
            res[j][i] = table[digits[i] - '2'][letterIndex];
        }
    }
    
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    char* digits;
    char** expected;
    int expectedSize;
} TestCase;

static int cmp_str(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

static void run_test(TestCase tc) {
    int returnSize = 0;
    char** result = letterCombinations(tc.digits, &returnSize);

    int pass = (returnSize == tc.expectedSize);

    if (pass && returnSize > 0) {
        qsort(result, returnSize, sizeof(char*), cmp_str);
        qsort(tc.expected, tc.expectedSize, sizeof(char*), cmp_str);

        for (int i = 0; i < returnSize; i++) {
            if (strcmp(result[i], tc.expected[i]) != 0) pass = 0;
        }
    }

    printf("[%s] digits=\"%s\" got %d combination(s), expected %d -> %s\n", tc.name, tc.digits, returnSize, tc.expectedSize, pass ? "PASS" : "FAIL");

    if (result != NULL) {
        for (int i = 0; i < returnSize; i++) free(result[i]);
        free(result);
    }
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
    char digits[] = "7777"; // 4 digits, worst-case branching (4 letters each)

    int iterations = 100000;
    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int returnSize = 0;
    char** result = NULL;
    for (int k = 0; k < iterations; ++k) {
        if (result != NULL) {
            for (int i = 0; i < returnSize; i++) free(result[i]);
            free(result);
        }
        result = letterCombinations(digits, &returnSize);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] digits=\"%s\", combinations=%d, avg over %d calls, elapsed=%.5f ms\n", digits, returnSize, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (letterCombinations call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif

    if (result != NULL) {
        for (int i = 0; i < returnSize; i++) free(result[i]);
        free(result);
    }
}

int main(void) {
    char d1[] = "23";
    char* expected1[] = {"ad", "ae", "af", "bd", "be", "bf", "cd", "ce", "cf"};

    char d2[] = "";
    char** expected2 = NULL;

    char d3[] = "2";
    char* expected3[] = {"a", "b", "c"};

    run_test((TestCase){"example 1", d1, expected1, 9});
    run_test((TestCase){"empty digits", d2, expected2, 0});
    run_test((TestCase){"single digit", d3, expected3, 3});

    benchmark();

    return 0;
}
