#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

void multiply(int mat[4]) {
    int x = mat[0] * mat[0] + mat[1] * mat[2];
    int y = mat[0] * mat[1] + mat[1] * mat[3];
    int z = mat[2] * mat[0] + mat[3] * mat[2];
    int w = mat[2] * mat[1] + mat[3] * mat[3];

    mat[0] = x;
    mat[1] = y;
    mat[2] = z;
    mat[3] = w;
}

void matrixPower(int mat[4], int n) {
    if (n == 0 || n == 1) return;
    matrixPower(mat, n / 2);

    int x = mat[0] * mat[0] + mat[1] * mat[2];
    int y = mat[0] * mat[1] + mat[1] * mat[3];
    int z = mat[2] * mat[0] + mat[3] * mat[2];
    int w = mat[2] * mat[1] + mat[3] * mat[3];

    mat[0] = x;
    mat[1] = y;
    mat[2] = z;
    mat[3] = w;

    if (n % 2 != 0) {
        x = mat[0];
        y = mat[2];
        mat[0] = mat[0] + mat[1];
        mat[1] = x;
        mat[2] = mat[2] + mat[3];
        mat[3] = y;
    }
}

int fib(int n) {
    if (n <= 1) return n;
    int mat[4] = { 1, 1, 1, 0 };
    matrixPower(mat, n);
    return mat[2];
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int n;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = fib(tc.n);
    int pass = got == tc.expected;
    printf("[%s] fib(%d) got=%d, expected=%d -> %s\n", tc.name, tc.n, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int n = 40;
    int iterations = 100000;

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = fib(n);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] n=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", n, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (fib call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main() {
    // run_test((TestCase){"n = 0", 0, 0});
    // run_test((TestCase){"n = 1", 1, 1});
    // run_test((TestCase){"n = 2", 2, 1});
    // run_test((TestCase){"n = 3", 3, 2});
    // run_test((TestCase){"n = 4", 4, 3});
    // run_test((TestCase){"n = 5", 5, 5});
    // run_test((TestCase){"n = 10", 10, 55});
    run_test((TestCase){"n = 20", 20, 6765});
    run_test((TestCase){"n = 30", 30, 832040});

    benchmark();

    return 0;
}