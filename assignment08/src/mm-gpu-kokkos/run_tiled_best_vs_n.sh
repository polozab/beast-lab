#!/usr/bin/env bash
set -euo pipefail

# Best four configs observed
CONFIGS=(
  "64 16 64 16"
  "64 32 64 16"
  "128 16 128 8"
  "128 8 128 8"
)

# Sweep N from 256 to 10000 in steps
SIZES=(256 512 1024 2048 3072 4096 5120 6144 7168 8192 9216 10000)

OUT_FILE="tiled_best_vs_n.txt"
echo "Best tiled configs across N" > "$OUT_FILE"
echo "N tile_m tile_n team vec time_s gflops" >> "$OUT_FILE"

export OMP_PROC_BIND=spread
export OMP_PLACES=threads

for N in "${SIZES[@]}"; do
  for cfg in "${CONFIGS[@]}"; do
    read -r TM TN TS VL <<<"$cfg"
    echo "Running N=${N} tile_m=${TM} tile_n=${TN} team=${TS} vec=${VL}"
    output=$(./kokkos_mm_TT_tiled "$N" "$TM" "$TN" "$TS" "$VL")
    time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
    gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
    echo "${N} ${TM} ${TN} ${TS} ${VL} ${time_s} ${gflops}" >> "$OUT_FILE"
  done
done

echo "Results written to ${OUT_FILE}"
