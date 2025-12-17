#!/usr/bin/env bash
set -euo pipefail

# Fixed problem size
N=6144

# Output file
OUT_FILE="tiled_results_${N}.txt"
echo "Layout: RRR (tiled), N=${N}" > "$OUT_FILE"
echo "tile_m tile_n team_size vec_len time_s gflops" >> "$OUT_FILE"

# Set OpenMP placement for consistency
export OMP_PROC_BIND=spread
export OMP_PLACES=threads

# tile_m tile_n team_size vec_len (only combos with team_size*vec_len <= 1024)
COMBOS=(
  "64 8 64 8"
  "64 8 64 4"
  "128 16 64 16"
  "128 16 64 8"
  "128 16 128 8"
  "128 8 64 4"
  "128 8 64 8"
  "128 16 64 4"
  "128 16 128 4"
)

for combo in "${COMBOS[@]}"; do
  read -r TM TN TS VL <<<"$combo"
  echo "Running tile_m=${TM} tile_n=${TN} team=${TS} vec=${VL}"
  output=$(./kokkos_mm_TT_tiled "$N" "$TM" "$TN" "$TS" "$VL")
  time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
  gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
  echo "${TM} ${TN} ${TS} ${VL} ${time_s} ${gflops}" >> "$OUT_FILE"
done

echo "Results written to ${OUT_FILE}"
