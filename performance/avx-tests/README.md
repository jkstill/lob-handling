
This will build on Linux x86.  

The AVX code is included only if the CPU supports it.


gcc -O3 -Wall -Wextra -mavx512bw -mavx512vl  -DTEST_HEX hexsimd.c -o hexsimd
HEXSIMD_FORCE=scalar ./hextest
HEXSIMD_FORCE=avx2 ./hextest

./hexsimd-tests.sh

==================================

ISA Extensions Explained
===================================
ISA = Instruction Set Architecture = the set of instructions a CPU can execute.

SIMD extensions are sub-ISAs

SSE2, AVX2, or AVX-512, are all extensions to the base x86-64 ISA — new instruction sets that add wider vector registers and new operations.

SIMD Extension   Year   Vector Width   Notes
SSE (SSE, SSE2, SSE3)   ~1999-2004   128-bit   Legacy SIMD foundation
AVX / AVX2   ~2011-2013   256-bit   Wider vectors, 3-operand form
AVX-512   ~2016+   512-bit   Very wide, mask registers, lots of variants

Each new SIMD “ISA extension” means new instructions the CPU can execute — e.g., _mm512_add_epi8 is an AVX-512 instruction, _mm_add_epi8 is an SSE2 instruction.

If a CPU doesn’t support that sub-ISA, and the binary tries to execute such an instruction, you get an illegal instruction.

This code:

The library supports multiple ISAs (or ISA extensions) for the same algorithm:

Scalar path → uses the base x86-64 ISA (works everywhere)

SSE2 path → x86-64 + SSE2 instructions (128-bit vectors)

AVX2 path → x86-64 + AVX2 (256-bit vectors)

AVX-512 path → x86-64 + AVX-512BW/VL (512-bit vectors)

At runtime, the code detects which ISA extensions the CPU supports and dispatches to the best implementation.

===================================

make clean && make -j

tests/test_hexsimd

./hexsimd-tests.sh

make selftest

===================================

Build without AVX-512 implementations:
make NO_AVX512=1

Build without AVX2 implementations:
make NO_AVX2=1

Slightly better tuning for the local CPU (still baseline ISA):
make NATIVE=1

comparison of SIMD register widths and parallelism across ISAs, showing why AVX512 can process 64 bytes in one shot where scalar only handles one.

🧮 SIMD register width comparison
              (Each box = 1 byte of data)

🟩 Scalar (no SIMD)
┌────────────┐
│ 1 byte     │  ← operates on a single byte per instruction
└────────────┘


➡ Works everywhere, slowest (1 byte/step).

🟦 SSE2 (Streaming SIMD Extensions 2)
┌────────────────────────────────────────────────────────────┐
│ 16 bytes = 128 bits → XMM0–XMM15                          │
└────────────────────────────────────────────────────────────┘


➡ 16× parallel operations per instruction.

Registers: XMM0 … XMM15

🟨 AVX2 (Advanced Vector Extensions 2)
┌────────────────────────────────────────────────────────────────────────────┐
│ 32 bytes = 256 bits → YMM0–YMM15                                           │
└────────────────────────────────────────────────────────────────────────────┘


➡ 32× parallel operations per instruction.

Registers: YMM0 … YMM15

(Each YMM overlaps its corresponding XMM — they’re an extended version.)

🟥 AVX-512
┌────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
│ 64 bytes = 512 bits → ZMM0–ZMM31                                                                              │
└────────────────────────────────────────────────────────────────────────────────────────────────────────────┘


➡ 64× parallel operations per instruction — the wide highway of SIMD.

Registers: ZMM0 … ZMM31, plus mask registers (k0–k7) for per-lane predication.

SIMD throughput summary

ISA       Register size   Parallel bytes   Typical CPU year   Notes
Scalar        8–64 bits          1                   Always   Baseline
SSE2           128 bits         16                    2001+   Minimum for x86-64
AVX            256 bits         32                    2011+   First 256-bit ops
AVX2           256 bits         32                    2013+   Integer SIMD fully supported
AVX-512        512 bits         64                    2016+   Masking + many subextensions

===================================

build-and-demo.sh


