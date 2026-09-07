// An implementation of Dynamic Time Warping algorithm
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

typedef struct {
    int i; int j;
} Point;

double dtw(double* arr1, int len1, double* arr2, int len2, double q, double* mat) {
    for (int i = 0; i < len1; ++i) {
        for (int j = 0; j < len2; ++j) {
            mat[i * len2 + j] = pow(fabs(arr1[i] - arr2[j]), q);

            if (i > 0 || j > 0) {
                double down = (i > 0) ? mat[(i - 1) * len2 + j] : INFINITY;
                double right = (j > 0) ? mat[i * len2 + j - 1] : INFINITY; 
                double diag = (i > 0 && j > 0) ? mat[(i - 1) * len2 + j - 1] : INFINITY;

                mat[i * len2 + j] += fmin(down, fmin(right, diag));
            }
        }
    }

    return pow(mat[(len1 - 1) * len2 + len2 - 1], 1.0 / q);
}

int dtw_path(int len1, int len2, double* mat, Point* path) {
    int i = len1 - 1, j = len2 - 1;
    int len = 0;
    path[len++] = (Point) {i , j};

    while (i > 0 || j > 0) {
        if (i == 0) {
            j--;
        } else if (j == 0) {
            i--;
        } else {
            double down = mat[(i - 1) * len2 + j];
            double right = mat[i * len2 + (j - 1)];
            double diag = mat[(i - 1) * len2 + (j - 1)];
           
            if (diag <= down && diag <= right) {
                i--;
                j--;
            } else if (down <= right) {
                i--;
            } else {
                j--;
            }
           
            path[len++] = (Point){i, j};
        }
    }

    return len;
}

/* ---------- test harness ---------- */

/*
 * DTW does a chain of additions and one pow(), so an exact == is brittle.
 * 1e-9 relative is far tighter than any rounding yet catches real logic bugs.
 */
static int close_enough(double got, double want) {
    double scale = fabs(want) > 1.0 ? fabs(want) : 1.0;
    return fabs(got - want) <= 1e-9 * scale;
}

static int tests_run = 0, tests_failed = 0;

static void check_dtw(const char* name, double* a, int la, double* b, int lb,
                      double q, double expected) {
    double* mat = malloc(sizeof(double) * la * lb);
    double got = dtw(a, la, b, lb, q, mat);
    int pass = close_enough(got, expected);
    tests_run++;
    if (!pass) tests_failed++;
    printf("[%-28s] got=%g expected=%g : %s\n",
           name, got, expected, pass ? "PASS" : "FAIL");
    free(mat);
}

/* Verifies dtw_path returns the expected (i,j) cells, in the order dtw_path
 * stores them (endpoint first, walking back toward the origin). */
static void check_path(const char* name, double* a, int la, double* b, int lb,
                       double q, Point* want, int want_len) {
    double* mat = malloc(sizeof(double) * la * lb);
    dtw(a, la, b, lb, q, mat);
    Point* path = malloc(sizeof(Point) * (la + lb));
    int len = dtw_path(la, lb, mat, path);
    int pass = (len == want_len);
    for (int k = 0; pass && k < len; ++k)
        if (path[k].i != want[k].i || path[k].j != want[k].j) pass = 0;
    tests_run++;
    if (!pass) tests_failed++;
    printf("[%-28s] len=%d expected=%d : %s\n",
           name, len, want_len, pass ? "PASS" : "FAIL");
    free(path);
    free(mat);
}

int main(void) {
    /* identical sequences -> zero cost */
    double id[] = {1, 2, 3};
    check_dtw("identical q=1", id, 3, id, 3, 1.0, 0.0);
    check_dtw("identical q=2", id, 3, id, 3, 2.0, 0.0);

    /* single element each: just the local distance */
    double s1[] = {5}, s2[] = {3};
    check_dtw("single elem q=1", s1, 1, s2, 1, 1.0, 2.0);
    check_dtw("single elem q=2", s1, 1, s2, 1, 2.0, 2.0);  /* sqrt(2^2) */

    /* length-2 exact match */
    double t1[] = {1, 2};
    check_dtw("len2 exact match", t1, 2, t1, 2, 1.0, 0.0);

    /* perfect time-warp: repeats in a align 1:1 to b -> zero cost */
    double w1[] = {1, 1, 2, 3, 3}, w2[] = {1, 2, 3};
    check_dtw("perfect warp q=1", w1, 5, w2, 3, 1.0, 0.0);
    check_dtw("perfect warp q=2", w1, 5, w2, 3, 2.0, 0.0);

    /* ramp vs flat, hand-computed cost matrix -> 3 */
    double r1[] = {0, 1, 2}, r2[] = {0, 0, 0};
    check_dtw("ramp vs flat q=1", r1, 3, r2, 3, 1.0, 3.0);

    /* asymmetric lengths */
    double a1[] = {1, 3}, a2[] = {2};
    check_dtw("2x1 q=2", a1, 2, a2, 1, 2.0, sqrt(2.0));

    /* symmetry: dtw(a,b) == dtw(b,a) */
    check_dtw("symmetry check", r2, 3, r1, 3, 1.0, 3.0);

    /* path through the ramp-vs-flat matrix stays on the diagonal */
    Point want[] = {{2, 2}, {1, 1}, {0, 0}};
    check_path("ramp vs flat diagonal path", r1, 3, r2, 3, 1.0, want, 3);

    printf("\n%d/%d passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}