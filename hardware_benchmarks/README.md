# Hardware benchmarks

Reproductions of the benchmarks from Timur Doumler's talk on how hardware
(caches, alignment, branch prediction, SIMD, denormals) affects C++
performance in practice — [video](https://www.youtube.com/watch?v=a12jYibw0vs).
Each file is a standalone `main()` — build and run individually, or use
`build.sh` to build them all.

The talk's own warning applies: these are micro-benchmarks, and micro-
benchmarks are dangerous (confirmation bias, optimizers deleting the
thing you meant to measure). Treat the numbers as illustrations of the
effect, not as guarantees — actual ratios depend heavily on CPU, compiler,
and flags. Always profile real code before acting on a micro-benchmark.

Built and verified against an AMD Ryzen 5 5600H with g++ (MinGW) 12.2.0
at `-O3 -march=native`. All 10 benchmarks reproduce their intended
effect on this setup; two (`03`, `07`) needed real fixes to get there,
described below. If a result on your machine looks flat or backwards,
read the "caveats" comment at the top of that file before assuming the
code is wrong — several of these are genuinely hardware/compiler
sensitive, and the files say so honestly rather than asserting a number
that didn't happen here.

## The benchmarks

### 01 — Row-major vs column-major traversal
A 2D array (`vector<vector<int>>`) is really one long strip of memory,
row by row. Walking it in row-major order reads that strip
sequentially; walking it in column-major order jumps by a full row's
width on every single access. CPUs are built to stream contiguous
memory efficiently and are much worse at scattered jumps, so the "wrong
order" traversal is dramatically slower once the array outgrows the
cache. Observed: ~19x slowdown (the talk saw 30-40x).

### 02 — Cache-line stride
Memory is fetched from DRAM in whole 64-byte cache lines, not single
ints. Touching every element of a large array vs. every 16th element
(one int per cache line) should, in the idealized model, cost about the
same, since the same number of cache lines get pulled in either way —
the saving only kicks in once your stride skips whole lines entirely.
In practice, on hardware with a strong prefetcher, you'll more likely
see a continuous decline rather than a clean flat-then-drop curve:
small strides mean vastly more loop iterations (134M vs 8M in this
run), so plain instruction overhead matters even once the prefetcher
is hiding most of the memory latency. The underlying point still
holds — cost tracks distinct cache lines touched, not element count.

### 03 — Cache associativity
Caches aren't fully associative — they're split into a fixed number of
sets, each of which can only hold a small, fixed number of cache lines
("ways") at once. If your access stride happens to be a multiple of
`cache_size / ways`, every address you touch lands in the *same* set,
so only `ways` of them fit at a time no matter how much space the
cache has overall — they keep evicting each other.

This targets **L1D** specifically (32KB, 8-way, 64-byte lines on this
Zen3 chip), not L2/L3, for a concrete reason: L1D's set-index needs
exactly 12 address bits (6 index + 6 offset), which is exactly one 4KB
page — and the low 12 bits of a virtual address are guaranteed to
match the physical address on any normal page. L2/L3 need more than 12
bits, meaning the extra bits come from the physical page number, which
a plain heap allocation doesn't control — so a naive attempt to
reproduce the *same* effect on L2 is flaky by design (would need huge
pages / `mmap` with physically contiguous memory). L1D's conflict
period on this chip is 32KB / 8 = 4KB = 1024 ints; observed strides of
1024 and 2048 spike to ~175-190ms vs. ~125-140ms for neighbors one
element off.

### 04 — Sequential vs strided vs random access
Three ways to touch the same large array. Sequential reuses every byte
of a fetched cache line (16 ints per fetch). Strided-by-16 fetches a
fresh cache line on every touch, same as random — but unlike random,
its stride is *constant*, so the hardware prefetcher can detect the
pattern and start fetching ahead of time. Random defeats the
prefetcher entirely. So the useful comparison isn't strided-vs-
sequential (that gap is mostly about line reuse) — it's strided-vs-
random, which isolates what the prefetcher actually buys you.

### 05 — Branch prediction: sorted vs unsorted `count_if`
Counting how many elements of a `vector<float>` are `> 0` should do
the same amount of work whether the data is sorted or randomly
shuffled — same comparisons, same additions. But if the compiler emits
an actual conditional branch for that comparison, a random true/false
sequence is the one thing a branch predictor can never learn, while a
sorted array's branch outcome is trivially predictable (always false,
then always true). MSVC tends to emit a real branch here (and the talk
measured ~6x); GCC/Clang tend to emit a branchless `cmov`/select
instead, in which case there's nothing for a predictor to get wrong
and the ratio sits near 1x — which is exactly what this run shows.
Check the generated assembly (`-S`) to see which your compiler chose.

### 06 — Virtual dispatch and branch misprediction
The often-repeated claim that "virtual functions are slow" is usually
about the wrong cost. The vtable indirection itself is cheap. What's
expensive is the *branch* the CPU must predict to know which override
it's about to jump to — and unlike `05`, no compiler can optimize this
away, because it's genuine runtime polymorphism. Calling 10,000 `Dog`s
then 10,000 `Cat`s in a row lets the branch predictor learn the
pattern trivially; shuffling them so the concrete type is unpredictable
call-to-call pays a real misprediction cost on *every* call. Observed:
3.2x slower shuffled, on every compiler.

### 07 — Breaking a dependency chain
An earlier version of this file tried to reproduce a different part of
the talk: a two-array recurrence (`a[i] = b[i-1]*2; b[i] = a[i]+1`)
that a specific loop-restructuring trick supposedly let the compiler
auto-vectorize. That didn't hold up — `b[i]` genuinely depends on
`b[i-1]` no matter how the source is arranged, and no compiler can
vectorize a real serial recurrence like that (confirmed: neither GCC
nor Clang did, for either arrangement).

This version instead reproduces the talk's *other* dependency-chain
example — an atoi-style accumulator, credited there to Andrei
Alexandrescu's "Writing Fast Code" talk — with a case that's actually
fixable: summing an array with one accumulator forces every add to
wait for the previous add's result (bounded by the CPU's add
*latency*, ~3-4 cycles); summing with four independent accumulators
lets four adds overlap in the pipeline (bounded by add *throughput*
instead), combined into one value only at the very end. Verified via
`objdump` that neither version auto-vectorizes (zero `vaddps`, only
scalar `vaddss`), so the ~3.4x observed speedup is purely from
instruction-level parallelism, not an accidental SIMD win — matching
the talk's own ~3x figure.

### 08 — SIMD alignment
A multiply-add loop over two float arrays (`dst[i] += src[i] * gain`)
vectorizes cleanly when both arrays start on a SIMD-register-width
boundary. Offsetting one array by a single element doesn't make the
access illegal or "unaligned" in the crash-inducing sense — it just no
longer lines up with the SIMD register width, which costs extra
shuffle/blend work to compensate. Observed ~1.4x slower offset (the
talk saw ~20% on a recent chip at the time, up to 2.5x on an older
one — so this varies a lot by CPU generation).

### 09 — False sharing
Cache *lines*, not individual variables, are the unit of coherence
between cores. Four threads each incrementing their own, completely
independent counter can still be dramatically slower running in
parallel than running sequentially one after another — if those four
counters happen to land on the same 64-byte cache line, every write
from one core invalidates the other cores' cached copies of that line,
even though no actual data is shared between the threads. Padding each
counter out to its own cache line (`alignas(64)`) fixes it entirely.
Observed: unpadded parallel is 32x *slower* than sequential (severe
false sharing); padded parallel is 3-4x *faster* than sequential
(genuine, expected multicore benefit). Note: this needed `volatile`
counters to get a meaningful result at all — without it, the compiler
correctly proves the counters are never read and deletes the entire
loop, silently reporting ~0ms for everything.

### 10 — Denormal floats
Repeatedly multiplying a float by a number close to 1 is fast for
normal values, `inf`, `nan`, and `-0` — but if the value is a denormal
(exponent bits all zero, non-zero fraction), older FPUs fall back to a
slow microcoded path, sometimes ~30x slower. This shows up in real
audio code as a signal decays toward (but never quite reaches) zero,
e.g. in a reverb tail or feedback loop. On this machine the result is
~1x — a real result, not a bug: many recent Intel/AMD chips have
substantially fixed the slow denormal path for scalar float ops that
older CPUs had. Fix in code that does need it: flush-to-zero /
denormals-are-zero (`_MM_SET_FLUSH_ZERO_MODE`,
`_MM_SET_DENORMALS_ZERO_MODE`, from `<xmmintrin.h>`/`<pmmintrin.h>`).

## References

- Timur Doumler, the talk this repo reproduces —
  [video](https://www.youtube.com/watch?v=a12jYibw0vs)
- Google Benchmark, recommended over hand-rolled timers for anything
  beyond quick illustration — <https://github.com/google/benchmark>
- Igor Ostrovsky, "Gallery of Processor Cache Effects" (C examples of
  several of the same effects, e.g. `02`/`03`) —
  <https://igoro.com/archive/gallery-of-processor-cache-effects/>
- Herb Sutter, "Effective Concurrency: Eliminate False Sharing" (the
  article behind `09`) —
  <https://herbsutter.com/2009/05/15/effective-concurrency-eliminate-false-sharing/>
- Andrei Alexandrescu, "Writing Fast Code I / II" (CppCon 2019 — the
  accumulator-chain idea behind `07`); search the CppCon YouTube
  channel for the exact talks, since video IDs aren't repeated here on
  low confidence
- Klaus Iglberger, "Taming the Performance Beast" (Meeting C++,
  benchmarks various STL containers — mentioned in the talk re:
  `list`/`map` insertion patterns); search the Meeting C++ YouTube
  channel for the exact talk. Name reconstructed from the talk's audio
  transcript ("Claus eigelberger") — worth double-checking directly.

## Build

```
./build.sh
```

or individually, e.g.:

```
g++ -O3 -std=c++17 -march=native 01_row_vs_column_major.cpp -o 01.exe
```

`09_false_sharing.cpp` additionally needs `-lpthread`.

Notes:
- `-march=native` matters for `08` (SIMD width); without it the compiler
  may target a generic baseline that doesn't show the effect.
- Results vary by machine/compiler — the talk itself found different
  numbers on different CPU generations, and different codegen between
  Clang/GCC/MSVC for `05`.
- If you're on a different CPU, `03`'s `kConflictStride` is derived from
  this chip's L1D geometry (32KB / 8-way / 64B lines = 1024-int period);
  recompute it for yours as `(L1D size in bytes / ways) / sizeof(int)`.
