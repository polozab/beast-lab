#!/usr/bin/env bash
set -euo pipefail

# Sweep GPU-only matmul over a range of N values.
# Usage: ./run_gpu_only.sh [N1 N2 ...]; if no args, use a default set up to 10000.

if (( "$#" > 0 )); then
  SIZES=("$@")
else
  SIZES=(256 512 1024 2048 3072 4096 5120 6144 7168 8192 9216 10000)
fi

export OMP_PROC_BIND=spread
export OMP_PLACES=threads

OUT_FILE="gpu_only_results.txt"
echo "GPU-only sweep" > "$OUT_FILE"
echo "N time_s gflops" >> "$OUT_FILE"

for N in "${SIZES[@]}"; do
  echo "Running kokkos_mm_GPU_Only N=${N}"
  output=$(./kokkos_mm_GPU_Only "${N}")
  time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
  gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
  echo "${N} ${time_s} ${gflops}" >> "$OUT_FILE"
done

echo "Results written to ${OUT_FILE}"
