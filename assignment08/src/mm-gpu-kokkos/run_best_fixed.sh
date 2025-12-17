#!/usr/bin/env bash
set -euo pipefail

# Run best-known configs for each layout variant at N=6144
N=6144

declare -a COMMANDS=(
  "./kokkos_mm_TT_RRR ${N} 64 16 64 16"
  "./kokkos_mm_TT_RRL ${N} 64 16 64 16"
  "./kokkos_mm_TT_RLR ${N} 64 16 64 16"
  "./kokkos_mm_TT_RLL ${N} 64 16 64 16"
  "./kokkos_mm_TT_LRR ${N} 64 16 64 16"
  "./kokkos_mm_TT_LRL ${N} 64 16 64 16"
  "./kokkos_mm_TT_LLR ${N} 64 16 64 16"
  "./kokkos_mm_TT_LLL ${N} 64 16 64 16"
)

OUT_FILE="best_layouts_${N}.txt"
echo "Layout runs at N=${N}" > "$OUT_FILE"
echo "layout tile_m tile_n threads vec time_s gflops" >> "$OUT_FILE"

export OMP_PROC_BIND=spread
export OMP_PLACES=threads

for cmd in "${COMMANDS[@]}"; do
  layout=$(echo "$cmd" | awk '{print $1}')
  args=($cmd)
  tile_m=${args[2]}
  tile_n=${args[3]}
  threads=${args[4]}
  vec=${args[5]}

  echo "Running ${cmd}"
  output=$($cmd)
  time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
  gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
  echo "${layout} ${tile_m} ${tile_n} ${threads} ${vec} ${time_s} ${gflops}" >> "$OUT_FILE"
done

echo "Results written to ${OUT_FILE}"
