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
# Fresh set distinct from previous runs (team_size * vec_len <= 1024)
COMBOS=(
  "40 8 80 8"
  "40 16 80 8"
  "52 8 104 8"
  "52 16 104 8"
  "56 8 112 8"
  "56 16 112 8"
  "68 8 68 8"
  "68 16 68 8"
  "76 8 76 8"
  "76 16 76 8"
  "84 8 84 8"
  "84 16 84 8"
  "88 8 88 8"
  "88 16 88 8"
  "92 8 92 8"
  "92 16 92 8"
  "100 8 100 8"
  "100 16 100 8"
  "108 8 108 8"
  "108 16 108 8"
  "116 8 116 8"
  "116 16 116 8"
  "124 8 124 8"
  "124 16 124 8"
  "132 8 132 6"
  "132 16 132 6"
  "136 8 136 6"
  "136 16 136 6"
  "140 8 140 6"
  "140 16 140 6"
  "148 8 148 6"
  "148 16 148 6"
  "152 8 152 6"
  "152 16 152 6"
  "156 8 156 6"
  "156 16 156 6"
  "168 8 168 6"
  "168 16 168 6"
  "172 8 172 5"
  "172 16 172 5"
  "180 8 180 5"
  "180 16 180 5"
  "184 8 184 5"
  "184 16 184 5"
  "188 8 188 5"
  "188 16 188 5"
  "196 8 196 5"
  "196 16 196 5"
  "204 8 204 5"
  "204 16 204 5"
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
