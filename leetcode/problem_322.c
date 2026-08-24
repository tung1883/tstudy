#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

/*
 * Key notes:
 * - This is a very naive DP approach to this problem
 * - A better solution should add 2 things
 * 1. greedy heuristics: sort the coins -> move the bigger coins to front
 * 2. branch and bound -> cut off any nodes that have worse solution than the current one
 */

/*
 * Problem 322: Coin Change
 * https://leetcode.com/problems/coin-change/
 *
 * Input:
 *   coins     - array of distinct coin denominations (an unlimited
 *               supply of each), 1 <= coinsSize <= 12, 1 <= coins[i] <= 2^31 - 1
 *   amount    - target amount of money, 0 <= amount <= 10^4
 *
 * Output:
 *   Fewest number of coins needed to make up amount, or -1 if amount
 *   cannot be made up by any combination of the coins.
 *
 * Examples:
 *   coins = [1,2,5], amount = 11 -> 3   (5 + 5 + 1)
 *   coins = [2],      amount = 3 -> -1  (odd amount, only even coin)
 *   coins = [1],       amount = 0 -> 0
 */

int dp(int* coins, int coinsSize, int amount, int* arr) {
    if (amount < 0) return -1;
    if (amount == 0) return 0;
    
    int res = -1;
    if (arr[amount] != -2) return arr[amount];
    
    for (int i = 0; i < coinsSize; ++i) {
        int temp = dp(coins, coinsSize, amount - coins[i], arr);
        if (temp == -1) continue;
        if (res == -1 || res > temp + 1) res = temp + 1;
    }

    arr[amount] = res;
    return res;
};

int coinChange(int* coins, int coinsSize, int amount) {
    int* arr = malloc(sizeof(int) * (amount + 1));
    for (int i = 0; i <= amount; i++) {
        arr[i] = -2;
    }
    
    int res = dp(coins, coinsSize, amount, arr);
    free(arr);
    return res;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    int* coins;
    int coinsSize;
    int amount;
    int expected;
} TestCase;

static void run_test(TestCase tc) {
    int got = coinChange(tc.coins, tc.coinsSize, tc.amount);
    int pass = got == tc.expected;
    printf("[%s] amount=%d -> got=%d, expected=%d -> %s\n", tc.name, tc.amount, got, tc.expected, pass ? "PASS" : "FAIL");
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
    int iterations = 1000;
    int coins[] = {1, 5, 10, 21, 25}; // classic worst-case set for greedy-fails demos
    int coinsSize = 5;
    int amount = 10000; // max constraint

    size_t before = current_working_set_bytes();
    double start = high_res_ms();
    int result = 0;
    for (int k = 0; k < iterations; ++k) {
        result = coinChange(coins, coinsSize, amount);
    }
    double end = high_res_ms();
    size_t after = current_working_set_bytes();

    double elapsed_ms = (end - start) / iterations;
    printf("\n[benchmark] amount=%d, result=%d, avg over %d calls, elapsed=%.5f ms\n", amount, result, iterations, elapsed_ms);
#ifdef _WIN32
    double delta_kb = (after >= before) ? (after - before) / 1024.0 : 0.0;
    printf("Working set delta (coinChange call only): %.2f KB\n", delta_kb);
    printf("Process working set (whole program, for reference): %.2f MB\n", after / (1024.0 * 1024.0));
#else
    printf("Memory benchmark only implemented for Windows in this file.\n");
#endif
}

int main(void) {
    int c1[] = {1, 2, 5};
    int c2[] = {2};
    int c3[] = {1};
    int c4[] = {186, 419, 83, 408};

    run_test((TestCase){"example 1", c1, 3, 11, 3});
    run_test((TestCase){"example 2 - impossible", c2, 1, 3, -1});
    run_test((TestCase){"example 3 - zero amount", c3, 1, 0, 0});
    run_test((TestCase){"large coins", c4, 4, 6249, 20});

    // benchmark();

    return 0;
}
