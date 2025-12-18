#!/usr/bin/env bash
set -euo pipefail

tile_m=64
tile_n=16
threads=64
vec=16

if (( "$#" > 0 )); then
  SIZES=("$@")
else
  SIZES=(256 512 1024 2048 3072 4096 5120 6144 7168 8192 9216 10000)
fi

export OMP_PROC_BIND=spread
export OMP_PLACES=threads

OUT_FILE="rlr_compare.txt"
echo "RLR compare (tile_m=${tile_m}, tile_n=${tile_n}, threads=${threads}, vec=${vec})" > "$OUT_FILE"
echo "N variant time_s gflops" >> "$OUT_FILE"

for N in "${SIZES[@]}"; do
  for exe in ./kokkos_mm_TT_RLR ./kokkos_mm_TT_RLR_dual_tiled; do
    echo "Running ${exe} N=${N}"
    output=$(${exe} "${N}" "${tile_m}" "${tile_n}" "${threads}" "${vec}")
    time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
    gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
    echo "${N} ${exe##./} ${time_s} ${gflops}" >> "$OUT_FILE"
  done
done

echo "Results written to ${OUT_FILE}"
