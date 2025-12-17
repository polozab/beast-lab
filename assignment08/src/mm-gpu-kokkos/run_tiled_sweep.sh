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
# Fresh set distinct from previous runs
COMBOS=(
  "48 16 96 8"
  "48 24 96 8"
  "72 24 72 8"
  "72 36 72 8"
  "96 32 96 8"
  "104 16 104 8"
  "104 32 104 4"
  "112 28 112 8"
  "120 20 120 8"
  "144 16 144 4"
  "144 24 144 6"
  "160 24 160 6"
  "176 16 176 4"
  "192 32 96 8"
  "192 32 128 6"
  "208 16 208 4"
  "224 14 112 8"
  "224 28 112 8"
  "240 12 120 8"
  "240 24 120 8"
  "256 20 128 6"
  "288 12 96 8"
  "288 24 96 8"
  "300 16 150 6"
  "320 12 160 6"
  "336 16 112 8"
  "352 14 88 8"
  "368 16 92 8"
  "384 12 128 6"
  "400 10 100 8"
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
