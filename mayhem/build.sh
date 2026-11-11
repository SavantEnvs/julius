#!/usr/bin/env bash
#
# mayhem/build.sh — builds the fuzz harnesses (sanitized) and Julius's own regression suite (normal flags).
# Runs inside the commit image (mayhem/Dockerfile) as `mayhem` in /mayhem; re-runnable and offline.
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"

# 1) The game core (upstream's own source lists, same file set as its `autopilot` test) with sanitizers,
#    DWARF 3 and SanCov, as one static library. mayhem/fuzz_project.cmake is injected at the end of
#    upstream's top-level CMakeLists.txt, so nothing upstream is edited. build-fuzz/ is gitignored (/build*/).
# shift-base: upstream reads little-endian ints with `byte << 24` into int everywhere (benign in practice, would flood every input)
FUZZ_CFLAGS="$SANITIZER_FLAGS -fno-sanitize=shift-base $DEBUG_FLAGS -fsanitize=fuzzer-no-link"
CC="$CC" cmake -S . -B build-fuzz -G Ninja \
    -DCMAKE_BUILD_TYPE=None \
    -DCMAKE_C_FLAGS="$FUZZ_CFLAGS" \
    -DCMAKE_PROJECT_INCLUDE="$SRC/mayhem/fuzz_hook.cmake"
cmake --build build-fuzz --target julius_fuzzcore -j"$MAYHEM_JOBS"
LIBCORE="$SRC/build-fuzz/libjulius_fuzzcore.a"

# 2) Harnesses. lsan_off.cc turns leak detection off at build time; the standalone binary is the
#    run-once reproducer for each harness.
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o /tmp/lsan_off.o
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o /tmp/standalone_main.o
for h in fuzz_savegame fuzz_scenario fuzz_empire fuzz_smacker fuzz_zip; do
    $CC $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE \
        -I"$SRC/src" -I"$SRC/mayhem/harnesses" "$SRC/mayhem/harnesses/$h.c" /tmp/lsan_off.o "$LIBCORE" -lm \
        -o "/mayhem/$h"
    $CC $SANITIZER_FLAGS $DEBUG_FLAGS \
        -I"$SRC/src" -I"$SRC/mayhem/harnesses" "$SRC/mayhem/harnesses/$h.c" /tmp/standalone_main.o /tmp/lsan_off.o "$LIBCORE" -lm \
        -o "/mayhem/$h-standalone"
done
rm -f /tmp/lsan_off.o /tmp/standalone_main.o

# 3) Julius's own regression suite (test/): normal flags, no sanitizers, independent build tree.
cmake -S . -B build-tests -G Ninja -DCMAKE_C_COMPILER="$CC" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="$COVERAGE_FLAGS"
cmake --build build-tests --target autopilot -j"$MAYHEM_JOBS"
