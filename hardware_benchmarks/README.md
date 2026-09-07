# Hardware benchmarks

Reproductions of the hardware-effects benchmarks from Timur Doumler's talk
([video](https://www.youtube.com/watch?v=a12jYibw0vs)). Each file is a standalone
`main()`. Build all with `./build.sh`, or one at a time:

```
g++ -O3 -std=c++17 -march=native 01_row_vs_column_major.cpp -o 01.exe
```

`09_false_sharing.cpp` also needs `-lpthread`.

Micro-benchmarks: numbers are illustrations, not guarantees. They depend on CPU,
compiler, and flags. Built and verified on an AMD Ryzen 5 5600H, g++ (MinGW)
12.2.0, `-O3 -march=native`. If your numbers look flat or backwards, read that
file's header comment; several effects are genuinely hardware/compiler-sensitive.

## 01. Row-major vs column-major traversal
**Problem:** a 2D array is one strip of memory, row by row. Row-major traversal
reads it sequentially; column-major jumps a full row width every access.
**Expected:** slower column-major, worse as the working set outgrows each cache
level (L1, L2, L3, DRAM). DRAM is the biggest single jump.
**Observed:** ~4x at L1-resident size (from vectorization, not misses: row-major
auto-vectorizes, column-major can't), up to ~34x at DRAM size. L2/L3 tiers don't
form a clean staircase, likely associativity noise (see `03`). L1-vs-DRAM
extremes are reliable.

## 02. Cache-line stride
**Problem:** touch every element of a large array vs. every 16th (one per 64-byte
line).
**Expected:** roughly flat cost up to stride 16 (same number of lines pulled),
drops once stride exceeds one line.
**Observed:** continuous decline rather than flat-then-drop. Small strides mean
more loop iterations, so instruction overhead shows through even with the
prefetcher hiding memory latency. Point holds: cost tracks distinct lines
touched, not element count.

## 03. Cache associativity
**Problem:** caches are split into sets holding a few lines each ("ways"). A
stride that is a multiple of `cache_size / ways` maps every address to the same
set.
**Expected:** sharp spike at that exact stride, not a smooth slope.
**Observed:** targets **L1D** (32KB, 8-way here), not L2/L3, because L1D's
set-index fits inside one 4KB page and is reproducible with a plain array. L2/L3
need physical-address bits a heap allocation doesn't control (would need huge
pages). Conflict period here is 1024 ints; strides of 1024/2048 spike to
~175-190ms vs. ~125-140ms one element off.

## 04. Sequential vs strided vs random access
**Problem:** sequential, constant-stride, and random access over the same large
array.
**Expected:** random much worse than strided even though both fetch a fresh line
per touch; the prefetcher locks onto a constant stride but not randomness.
**Observed:** matches. Strided-vs-sequential is not the interesting comparison:
that gap is mostly cache-line reuse, not prefetching.

## 05. Branch prediction: sorted vs unsorted `count_if`
**Problem:** count positives in a `vector<float>`, sorted vs. shuffled.
**Expected:** none if the compiler emits branchless code (`cmov`); large (talk
saw ~6x on MSVC) if it emits a real branch, since a random sequence is
unpredictable.
**Observed:** ~1x on GCC/Clang (branchless). Check `-S` output for your compiler.

## 06. Virtual dispatch and branch misprediction
**Problem:** sum virtual-call results over grouped (`Dog x10000, Cat x10000`) vs.
shuffled objects.
**Expected:** shuffled slower, not from vtable indirection but from branch
misprediction on which override to jump to, which no compiler can remove.
**Observed:** ~3.2x slower shuffled, on every compiler.

## 07. Breaking a dependency chain
**Problem:** sum an array with one accumulator vs. four independent accumulators.
**Expected:** one accumulator is bound by add *latency* (each add waits for the
last); four are bound by *throughput*, since the CPU overlaps them.
**Observed:** ~3.4x speedup with four, confirmed via `objdump` to be pure ILP
(zero SIMD in either version). An earlier version of this file used a recurrence
that doesn't vectorize under any restructuring (`b[i]` genuinely depends on
`b[i-1]`); this version uses the actually-fixable case.

## 08. SIMD alignment
**Problem:** multiply-add loop (`dst[i] += src[i] * gain`), buffers aligned vs.
offset by one element.
**Expected:** offset costs extra shuffle/blend work for not landing on a
SIMD-register boundary.
**Observed:** ~1.4x slower offset (varies a lot by CPU generation).

## 09. False sharing
**Problem:** four threads each incrementing their own counter, counters packed
together or padded to separate cache lines.
**Expected:** packed counters make parallel execution *slower* than sequential,
since every write invalidates the other cores' cached copies of that line,
despite no data being shared.
**Observed:** unpadded parallel is 32x slower than sequential; padded parallel is
3-4x faster. Needs `volatile` counters, else the compiler proves them unused and
deletes the loop, reporting ~0ms.

## 10. Denormal floats
**Problem:** repeatedly multiply a float close to 1, for a normal value, `inf`,
`nan`, `-0`, and a denormal.
**Expected:** denormals can be ~30x slower on older FPUs (slow microcoded path).
**Observed:** ~1x here, a real result. Many recent Intel/AMD chips have fixed
this. Fix for chips that still need it: flush-to-zero / denormals-are-zero
(`_MM_SET_FLUSH_ZERO_MODE` / `_MM_SET_DENORMALS_ZERO_MODE`, from
`<xmmintrin.h>` / `<pmmintrin.h>`).

## 11. Misaligned scalar access
**Problem:** scalar (non-SIMD) reads through pointers at byte offset 0 vs. offset
1. Offset 1 straddles a 64-byte cache line every 64 elements. Distinct from `08`,
which is SIMD register alignment.
**Expected:** a load that crosses a line boundary needs two line accesses stitched
internally, so offset should be slower.
**Observed:** ~1x (`int` 1.06x, `float` 0.97x). Modern x86 handles line-crossing
scalar loads with negligible penalty when the stream is prefetcher-friendly;
memory bandwidth dominates. Page-boundary crossings still cost more.

## References

- Timur Doumler, [talk this repo reproduces](https://www.youtube.com/watch?v=a12jYibw0vs)
- [Google Benchmark](https://github.com/google/benchmark), preferred over hand-rolled timers for real use
- Igor Ostrovsky, [Gallery of Processor Cache Effects](https://igoro.com/archive/gallery-of-processor-cache-effects/), relevant to `02`/`03`
- Herb Sutter, [Eliminate False Sharing](https://herbsutter.com/2009/05/15/effective-concurrency-eliminate-false-sharing/), relevant to `09`
- Andrei Alexandrescu, "Writing Fast Code I / II" (CppCon 2019), accumulator-chain idea behind `07`; exact video not linked, low confidence
- Klaus Iglberger, "Taming the Performance Beast" (Meeting C++), STL container benchmarks from the talk; name reconstructed from audio, worth double-checking

## Notes
- `-march=native` matters for `08` (SIMD width).
- `03`'s conflict stride comes from this chip's L1D geometry (32KB / 8-way / 64B
  lines = 1024-int period); recompute as `(L1D size / ways) / sizeof(int)` for
  another CPU.
