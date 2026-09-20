/* reference.c — host-side reference implementation of the BVLibs API.
 *
 * This file is the independent, portable companion to the kernels in
 * kernels/.  It implements the same API contract and the same math as
 * the ISA-bound runtime, directly on the host, so the test suite can run
 * anywhere (CI, porting, hardware bring-up) without the proprietary
 * toolchain.
 *
 * It is deliberately NOT the shipped runtime: it performs no assembly,
 * decoding, or warp simulation.  Sequential, single-threaded, portable C.
 */

#include "bvml.h"
#include "bvn.h"

#include <math.h>
#include <stdlib.h>

/* Shared scalar softpluses for GELU / SiLU. */
#define BVN_LOG2E 1.4426950408889634f   /* kept for kernel parity */

static const char *kBVMLVersion = "BVML 0.1.0 (host reference)";
static const char *kBVNVersion = "BVN 0.2.0 (host reference)";

/* ─── BVML ─────────────────────────────────────────────────────────────── */

struct bvml_context {
    size_t mem_bytes;
};

struct bvml_context *bvml_open(size_t mem_bytes)
{
    struct bvml_context *ctx = calloc(1, sizeof(*ctx));
    if (ctx)
        ctx->mem_bytes = mem_bytes;
    return ctx;
}

void bvml_close(struct bvml_context *ctx)
{
    free(ctx);
}

const char *bvml_version(void)
{
    return kBVMLVersion;
}

int bvml_saxpy(struct bvml_context *ctx, float a, const float *x,
               const float *y, float *out, size_t n)
{
    if (!ctx || !x || !y || !out || n == 0)
        return -1;
    for (size_t i = 0; i < n; i++)
        out[i] = a * x[i] + y[i];
    return 0;
}

int bvml_dot(struct bvml_context *ctx, const float *x, const float *y,
             float *out, size_t n)
{
    if (!ctx || !x || !y || !out || n == 0)
        return -1;
    float acc = 0.0f;
    for (size_t i = 0; i < n; i++)
        acc += x[i] * y[i];
    out[0] = acc;
    return 0;
}

/* ─── BVN ──────────────────────────────────────────────────────────────── */

struct bvn_context {
    size_t mem_bytes;
};

struct bvn_context *bvn_open(size_t mem_bytes)
{
    struct bvn_context *ctx = calloc(1, sizeof(*ctx));
    if (ctx)
        ctx->mem_bytes = mem_bytes;
    return ctx;
}

void bvn_close(struct bvn_context *ctx)
{
    free(ctx);
}

const char *bvn_version(void)
{
    return kBVNVersion;
}

int bvn_relu(struct bvn_context *ctx, const float *x, float *out, size_t n)
{
    if (!ctx || !x || !out || n == 0)
        return -1;
    for (size_t i = 0; i < n; i++)
        out[i] = x[i] > 0.0f ? x[i] : 0.0f;
    return 0;
}

int bvn_affine(struct bvn_context *ctx, float scale, float bias,
               const float *x, float *out, size_t n)
{
    if (!ctx || !x || !out || n == 0)
        return -1;
    for (size_t i = 0; i < n; i++)
        out[i] = scale * x[i] + bias;
    return 0;
}

int bvn_softmax(struct bvn_context *ctx, const float *x, float *out, size_t n)
{
    if (!ctx || !x || !out || n == 0)
        return -1;

    float m = x[0];
    for (size_t i = 1; i < n; i++)
        if (x[i] > m)
            m = x[i];

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        float e = (float)exp((double)(x[i] - m));
        out[i] = e;
        sum += (double)e;
    }
    for (size_t i = 0; i < n; i++)
        out[i] = (float)((double)out[i] / sum);
    return 0;
}

/* sigmoid approx: x / (1 + exp(-c*x)); c = 1.702 for GELU, c = 1 for SiLU.
 * exp(-z) is computed as exp2(-z*log2e) to mirror the kernel. */
static int sigm(const struct bvn_context *ctx, float c,
                const float *x, float *out, size_t n)
{
    if (!ctx || !x || !out || n == 0)
        return -1;
    for (size_t i = 0; i < n; i++) {
        double e = exp2((double)(-x[i] * c * BVN_LOG2E));
        out[i] = (float)((double)x[i] / (1.0 + e));
    }
    return 0;
}

int bvn_gelu(struct bvn_context *ctx, const float *x, float *out, size_t n)
{
    return sigm(ctx, 1.702f, x, out, n);
}

int bvn_silu(struct bvn_context *ctx, const float *x, float *out, size_t n)
{
    return sigm(ctx, 1.0f, x, out, n);
}