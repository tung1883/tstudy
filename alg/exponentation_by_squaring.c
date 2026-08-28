#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

// the idea: essentially, with a positive integer n, we have
// exp(x, n) = x * exp(x^2, (n - 1)/2) for n odd
// exp(x, n) = exp(x^2, n/2) for n even
// we can extend to the whole integer set Z by setting:
// 1. exp(x, 0) = 1
// 2. exp(x, n) = exp(1/x, -n) for n<0

// for non-negative n
double square_exp_pos(double x, unsigned int n) {
    if (n == 0) return 1.0;
    if (n % 2 == 0) return square_exp_pos(x * x, n / 2);
    return x * square_exp_pos(x * x, n / 2);
}

// recursive
double square_exp(double x, int n) {
    if (n < 0) return square_exp_pos(1.0 / x, -(unsigned int)n);
    return square_exp_pos(x, (unsigned int)n);
}

// iterative
double square_exp_iter(double x, int n) {
    unsigned int e = (n < 0) ? -(unsigned int)n : (unsigned int)n;
    if (n < 0) x = 1.0 / x;

    double result = 1.0;
    while (e > 0) {
        if (e & 1u) result *= x; // check if e even or odd
        x *= x;
        e >>= 1; // divide e by 2
    }
    return result;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    double x;
    int n;
    double expected;
} TestCase;

/*
 * Relative tolerance: exponentiation-by-squaring does ~log2(n) multiplies, each
 * rounding, so an exact == against pow() is too strict. 1e-9 is far tighter
 * than the accumulated error yet catches any real logic bug.
 */
static int close_enough(double got, double want) {
    double scale = fabs(want) > 1.0 ? fabs(want) : 1.0;
    return fabs(got - want) <= 1e-9 * scale;
}

static void run_test(TestCase tc) {
    double rec = square_exp(tc.x, tc.n);
    double itr = square_exp_iter(tc.x, tc.n);
    int pass = close_enough(rec, tc.expected) && close_enough(itr, tc.expected);
    printf("[%-18s] %g ^ %d -> rec=%g iter=%g (expected %g) : %s\n",
           tc.name, tc.x, tc.n, rec, itr, tc.expected,
           pass ? "PASS" : "FAIL");
}

/* ---------- benchmark ---------- */

/*
 * clock() only has ~1ms resolution on Windows, too coarse for a call this
 * short. QueryPerformanceCounter gives sub-microsecond resolution.
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

static double naive_pow(double x, int n) {
    double result = 1.0;
    int neg = n < 0;
    unsigned int e = neg ? (unsigned int)-(long)n : (unsigned int)n;
    for (unsigned int i = 0; i < e; i++)
        result *= x;
    return neg ? 1.0 / result : result;
}

/*
 * run_naive is a flag: naive_pow does |n| multiplies, so it's only meaningful
 * to time for small n. For large n we skip it rather than spin billions of
 * iterations.
 */
static void benchmark_case(const char* label, double x, int n, long iterations, int run_naive) {
    volatile double sink = 0.0;
    double start, rec_ms, iter_ms, naive_ms = -1.0;

    start = high_res_ms();
    for (long i = 0; i < iterations; i++) sink += square_exp(x, n);
    rec_ms = (high_res_ms() - start) / iterations;

    start = high_res_ms();
    for (long i = 0; i < iterations; i++) sink += square_exp_iter(x, n);
    iter_ms = (high_res_ms() - start) / iterations;

    if (run_naive) {
        start = high_res_ms();
        for (long i = 0; i < iterations; i++) sink += naive_pow(x, n);
        naive_ms = (high_res_ms() - start) / iterations;
    }

    if (run_naive)
        printf("[%-9s] rec=%.6f ms  iter=%.6f ms  naive=%.6f ms  (sink=%g)\n",
               label, rec_ms, iter_ms, naive_ms, (double)sink);
    else
        printf("[%-9s] rec=%.6f ms  iter=%.6f ms  naive=skipped   (sink=%g)\n",
               label, rec_ms, iter_ms, (double)sink);
}

static void benchmark(void) {
    long iterations = 200000;
    printf("\n[benchmark] avg over %ld calls per case\n", iterations);
    benchmark_case("n=2",       1.0000001, 2, iterations, 1);
    benchmark_case("n=15",      1.0000001, 15, iterations, 1);
    benchmark_case("n=1000",    1.0000001, 1000, iterations, 1);
    benchmark_case("n=-1000",   1.0000001, -1000, iterations, 1);
    benchmark_case("n=INT_MAX", 1.0000000001, INT_MAX, iterations, 0);
}

int main(void) {
    run_test((TestCase){"zero exponent",      5.0, 0, 1.0});
    run_test((TestCase){"one exponent",       5.0, 1, 5.0});
    run_test((TestCase){"base zero",          0.0, 3, 0.0});
    run_test((TestCase){"base one",           1.0, 1000000, 1.0});
    run_test((TestCase){"small even",         2.0, 10, 1024.0});
    run_test((TestCase){"small odd",          2.0, 11, 2048.0});
    run_test((TestCase){"negative base even", -2.0, 4, 16.0});
    run_test((TestCase){"negative base odd",  -2.0, 5, -32.0});
    run_test((TestCase){"negative exponent",  2.0, -3, 0.125});
    run_test((TestCase){"neg base neg exp",   -2.0, -3, -0.125});
    run_test((TestCase){"fractional base",    1.5, 8, 25.62890625});
    run_test((TestCase){"large exponent",     1.0000001, 1000, pow(1.0000001, 1000)});
    run_test((TestCase){"int_min exponent",   1.0, INT_MIN, 1.0});
    run_test((TestCase){"int_max exponent",   1.0, INT_MAX, 1.0});

    benchmark();
    return 0;
}
