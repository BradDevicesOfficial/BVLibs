#include "bvml.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int check(const char *name, float got, float want)
{
    float tol = 1e-4f * (fabsf(want) + 1.0f);
    if (fabsf(got - want) > tol) {
        printf("FAIL %s: got %.6g want %.6g\n", name, got, want);
        return 1;
    }
    return 0;
}

static int run_case(struct bvml_context *c, size_t n)
{
    float *x = malloc(n * sizeof(float));
    float *y = malloc(n * sizeof(float));
    float *out = malloc(n * sizeof(float));
    if (!x || !y || !out) {
        printf("FAIL malloc (n=%zu)\n", n);
        free(x);
        free(y);
        free(out);
        return 1;
    }
    for (size_t i = 0; i < n; i++) {
        x[i] = (float)(i + 1);
        y[i] = 2.0f * (float)(i + 1);
    }

    int bad = 0;

    if (bvml_saxpy(c, 0.5f, x, y, out, n) != 0) {
        printf("FAIL saxpy launch (n=%zu)\n", n);
        bad++;
    } else {
        for (size_t i = 0; i < n; i++)
            bad += check("saxpy", out[i], 0.5f * x[i] + y[i]);
    }

    float d = 0.0f;
    if (bvml_dot(c, x, y, &d, n) != 0) {
        printf("FAIL dot launch (n=%zu)\n", n);
        bad++;
    } else {
        float want = 0.0f;
        for (size_t i = 0; i < n; i++)
            want += x[i] * y[i];
        bad += check("dot", d, want);
    }

    free(x);
    free(y);
    free(out);
    return bad;
}

int main(void)
{
    const size_t sizes[] = { 1, 7, 64, 100 };
    const size_t ncases = sizeof(sizes) / sizeof(sizes[0]);

    struct bvml_context *c = bvml_open(1u << 20);
    if (!c) {
        printf("FAIL open — %s\n", bvml_version());
        return 1;
    }

    int bad = 0;
    for (size_t i = 0; i < ncases; i++)
        bad += run_case(c, sizes[i]);

    bvml_close(c);

    if (bad) {
        printf("BVML: %d check(s) FAILED\n", bad);
        return 1;
    }
    printf("BVML: all checks passed (saxpy + dot, n=1/7/64/100) — %s\n",
           bvml_version());
    return 0;
}
