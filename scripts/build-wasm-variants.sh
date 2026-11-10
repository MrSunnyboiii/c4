#!/usr/bin/env bash
#
# Builds the browser (WASM) solver module for every supported board size.
#
# Usage:
#   ./scripts/build-wasm-variants.sh [output-root] [sizes...]
#
#   output-root  Directory which will receive one `<width>x<height>/wasm/`
#                folder per board size (default: ui/variants).
#   sizes        Optional list of sizes to build, e.g. "7x5 4x8".
#                Defaults to all supported sizes.
#
# Requires an activated Emscripten environment (`source emsdk_env.sh`).
#
# Each build is scoped to its own module factory named createSolver<W>x<H>
# (e.g. createSolver7x5), so several sizes can be loaded by one page. No
# opening book is preloaded: the builds require no solver.data file.

set -euo pipefail

cd "$(dirname "$0")/.."

# Every rectangular board (width x height) with at least 4 columns, at least
# 4 rows, and at most 35 cells. Both orientations are built where both
# qualify, since gravity makes e.g. 4x8 and 8x4 different games.
ALL_SIZES=(4x4 5x4 6x4 7x4 8x4 4x5 5x5 6x5 7x5 4x6 5x6 4x7 5x7 4x8)

# Settings taken from the known working 7x5 browser build: a 64 MB
# transposition table (8388617 entries, must stay odd - see entry.h)
# inside 128 MB of initial memory. These satisfy the Entry::HASH_SPACE
# compile-time constraint for every size above; the static_asserts in
# entry.h re-verify this on every build.
NUM_TABLE_ENTRIES=8388617
INITIAL_MEMORY=134217728
NUM_THREADS=1

OUT_ROOT="${1:-ui/variants}"
shift || true

SIZES=("${@:-${ALL_SIZES[@]}}")

mkdir -p "${OUT_ROOT}"
OUT_ROOT="$(cd "${OUT_ROOT}" && pwd)"

for size in "${SIZES[@]}"; do
    width="${size%x*}"
    height="${size#*x}"
    build_dir="build/wasm-${size}"
    mkdir -p "${build_dir}"

    echo "=== Building ${width}x${height} -> ${OUT_ROOT}/${size}/wasm ==="

    emcmake cmake -S . -B "${build_dir}" -DMODE=WASM \
        -DBOARD_WIDTH="${width}" \
        -DBOARD_HEIGHT="${height}" \
        -DNUM_THREADS="${NUM_THREADS}" \
        -DNUM_TABLE_ENTRIES="${NUM_TABLE_ENTRIES}" \
        -DWASM_INITIAL_MEMORY="${INITIAL_MEMORY}" \
        -DWASM_EXPORT_NAME="createSolver${width}x${height}" \
        -DJS_OUTPUT="${OUT_ROOT}/${size}/wasm" \
        > "${build_dir}.configure.log" 2>&1 \
        || { cat "${build_dir}.configure.log"; exit 1; }

    cmake --build "${build_dir}" -j
done

echo "Built ${#SIZES[@]} variant(s) into ${OUT_ROOT}."
