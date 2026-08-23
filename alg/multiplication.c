#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Karatsuba multiplication, base 10 (digit-count split rather than bit
 * split, since it's meant to generalize to bignums later).
 */

uint64_t ipow(uint64_t a, unsigned int b)
{
    uint64_t result = 1;

    while (b > 0) {
        if (b & 1)
            result *= a;

        a *= a;
        b >>= 1;
    }

    return result;
}

const uint64_t BASE = 10;

int size_in_base(uint64_t num) {
    if (num == 0) return 1;

    int res = 0;
    while (num > 0) {
        res++;
        num /= 10;
    }

    return res;
}

/* res[0] = high digits, res[1] = low digits. Caller must free(). */
uint64_t* split_at(uint64_t num, unsigned int d) {
    uint64_t exp = ipow(BASE, d);
    uint64_t* res = malloc(2 * sizeof(uint64_t));
    res[0] = num / exp;
    res[1] = num % exp;
    return res;
}

uint64_t karatsuba(uint64_t num1, uint64_t num2) {
    if (num1 < BASE || num2 < BASE) return num1 * num2;

    int m = size_in_base(num1) > size_in_base(num2) ? size_in_base(num1) : size_in_base(num2);
    int m2 = m / 2;

    uint64_t* hl1 = split_at(num1, m2);
    uint64_t* hl2 = split_at(num2, m2);

    uint64_t z0 = karatsuba(hl1[1], hl2[1]);
    uint64_t z3 = karatsuba(hl1[0] + hl1[1], hl2[0] + hl2[1]);
    uint64_t z2 = karatsuba(hl1[0], hl2[0]);

    free(hl1);
    free(hl2);

    return z2 * ipow(BASE, m2 * 2) + ((z3 - z2 - z0) * ipow(BASE, m2)) + z0;
}

/* ---------- test harness ---------- */

typedef struct {
    const char* name;
    uint64_t a;
    uint64_t b;
    uint64_t expected;
} TestCase;

static void run_test(TestCase tc) {
    uint64_t result = karatsuba(tc.a, tc.b);
    int pass = result == tc.expected;
    printf("[%s] %llu * %llu -> %llu (expected %llu) : %s\n",
           tc.name,
           (unsigned long long)tc.a, (unsigned long long)tc.b,
           (unsigned long long)result, (unsigned long long)tc.expected,
           pass ? "PASS" : "FAIL");
}

int main(void) {
    run_test((TestCase){"single digits", 7, 8, 56});
    run_test((TestCase){"zero", 0, 12345, 0});
    run_test((TestCase){"one", 1, 999, 999});
    run_test((TestCase){"two digit", 12, 34, 408});
    run_test((TestCase){"four digit", 1234, 5678, 7006652});
    run_test((TestCase){"mixed width", 123, 45678, 5618394});
    run_test((TestCase){"large", 987654321ULL, 123456789ULL, 121932631112635269ULL});

    return 0;
}
