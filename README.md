# BVLibs

[![CI](https://github.com/BradDevicesOfficial/BVLibs/actions/workflows/ci.yml/badge.svg)](https://github.com/BradDevicesOfficial/BVLibs/actions/workflows/ci.yml)

The reference math and neural libraries of the BradVector platform: the C API,
the ISA kernels they are built on, and an independent host-side reference
implementation used for CI, porting and hardware bring-up.

| artifact | what it is |
|---|---|
| `include/` | the public `BVML` / `BVN` C API (kernel-backed on the platform) |
| `kernels/` | `.bvbs` ISA sources, written for the public BradVector ISA |
| `src/reference.c` | host-side reference implementation of the same API + math |
| `tests/` | the same test suite the platform runs in its own CI |

## Status

```text
SHIPPED   BVLibs API + kernels            — in this repository
SHIPPED   host-side reference (CI)        — in this repository
DESIGNED  production silicon core         — BradVector ISA + reference RTL
VISION    real hardware interconnects     — BradFusion fabric (reference model)
```

The included kernels are written against the [BradVector ISA](https://github.com/BradDevicesOfficial/BradVector)
and the platform's reference runtime executes them on a 32-lane warp. This
repository keeps the library boundary public; the proprietary toolchain
(assembler + warp simulator) ships with the BradDevices software platform and
is not part of this repo.

## Build & test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
```

Requires a C11 compiler (and `libm`). The tests validate `bvml_saxpy`,
`bvml_dot`, `bvml_gemm`, `bvn_relu`, `bvn_affine`, `bvn_softmax`, `bvn_gelu`
and `bvn_silu` against independent host math.

## API

```c
struct bvml_context *bvml_open(size_t mem_bytes);
int bvml_saxpy(struct bvml_context *ctx, float a, const float *x,
               const float *y, float *out, size_t n);   /* out = a*x + y */
int bvml_dot(struct bvml_context *ctx, const float *x, const float *y,
             float *out, size_t n);                     /* out[0] = x·y */
```

```c
struct bvn_context *bvn_open(size_t mem_bytes);
bvn_relu(a)      out[i] = max(0, x[i])
bvn_affine()     out[i] = scale*x[i] + bias
bvn_softmax()    stable max-subtracted softmax over n
bvn_gelu()       x / (1 + exp(-1.702*x))
bvn_silu()       x / (1 + exp(-x))
```

All operations return `0` on success and a negative code on bad arguments.

## Layout

```
include/    public headers
kernels/    ISA sources (the kernel-backed path on the platform)
src/        host-side reference implementation
tests/      shared test suite
```

## How it relates

- [brad-devices](https://github.com/BradDevicesOfficial/brad-devices) — the brand site; its
  in-browser demo runs these sources compiled to WebAssembly.
- [BradISA](https://github.com/BradDevicesOfficial/BradISA) — the CPU ISA.
- [BradVector](https://github.com/BradDevicesOfficial/BradVector) — the GPU/accelerator ISA,
  its public assembler text, and reference RTL.

## License

MIT — see [LICENSE](LICENSE).