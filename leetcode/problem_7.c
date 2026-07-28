#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - Don't try to create a marvelous algorithm for overflow(x), dumbass!
 * - Just check for overflow when you do the addition that can lead to it
 * - So the trick here is to before adding res = res * 10 + digit,
 * check if res > INT_MAX / 10 || (res == INT_MAX && digit > 7).
 * Same thing goes for negative one
 */

/*
 * Problem 7: Reverse Integer
 * https://leetcode.com/problems/reverse-integer/
 *
 * Input:
 *   x - 32-bit signed integer, -2^31 <= x <= 2^31 - 1
 *
 * Output:
 *   x with its digits reversed.
 *   Returns 0 if reversing overflows a 32-bit signed integer.
 */
int reverse(int x) {
    int res = 0;
    int digit = 0;
    
    while (x != 0) {
        digit = x % 10;
        
        // overflow
        if (res < -2147483648 / 10 || res == -2147483648 / 10 && digit > 8) return 0;
        if (res > 2147483647 / 10 || res == 2147483647 / 10 && digit > 7) return 0;
        res = res * 10 + digit;
        x = (x - digit) / 10;
    }
    
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int x;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = reverse(tc.x);
    int pass = got == tc.expected;
    printf("[%s] x=%d -> got=%d, expected=%d -> %s\n", tc.name, tc.x, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int x = 1534236469; // large 10-digit number, worst case for digit-reversal work

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = reverse(x);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] x=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", x, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (reverse call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    run_test((TestCase){"example 1", 123, 321});
    run_test((TestCase){"example 2", -123, -321});
    run_test((TestCase){"example 3", 120, 21});
    run_test((TestCase){"zero", 0, 0});
    run_test((TestCase){"overflow positive", 1534236469, 0});
    run_test((TestCase){"overflow negative", -2147483648, 0});
    run_test((TestCase){"single digit", 5, 5});

    benchmark();

    return 0;
}
