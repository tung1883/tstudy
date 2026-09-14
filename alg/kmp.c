#include <stdio.h>
#include <stdlib.h>

void buildlsp(char* pat, int m, int* lps) {
    lps[0] = 0;

    int len = 0;
    int i = 1;

    while (i < m) {
        if (pat[i] == pat[len]) lps[i++] = ++len;
        else {
            if (len != 0) len = lps[len - 1]; 
            else lps[i++] = 0;
        }
    }
}

int stringLen(char* str) {
    int i = 0;
    while (str[i] != '\0') i++;
    return i;
}

int kmp(char* text, char* pat) {
    int n = stringLen(text), m = stringLen(pat), lps[m], i = 0, j = 0;
    buildlsp(pat, m, lps);

    while (i < n) {
        if (text[i] == pat[j]) {
            i++; j++;

            if (j == m) return i - j;
        } else {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }

     return -1;
}

/* ---------- test harness ---------- */

static int tests_run = 0, tests_failed = 0;

static void check_kmp(const char* name, char* text, char* pat, int expected) {
    int got = kmp(text, pat);
    int pass = got == expected;
    tests_run++;
    if (!pass) tests_failed++;
    printf("[%-28s] got=%d expected=%d : %s\n", name, got, expected, pass ? "PASS" : "FAIL");
}

int main(void) {
    check_kmp("match at start", "abcabcabc", "abc", 0);
    check_kmp("match in middle", "xxabcxx", "abc", 2);
    check_kmp("match at end", "xxabc", "abc", 2);
    check_kmp("no match", "xxxxx", "abc", -1);
    check_kmp("pattern == text", "abc", "abc", 0);
    check_kmp("single char match", "aaaa", "a", 0);
    check_kmp("single char no match", "bbbb", "a", -1);
    check_kmp("overlapping pattern", "aaaaa", "aa", 0);
    check_kmp("classic lps case", "ababcababcabc", "abcabc", 7);
    check_kmp("pattern longer than text", "ab", "abc", -1);

    printf("\n%d/%d passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
