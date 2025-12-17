#!/usr/bin/env bash
set -euo pipefail

# Sweep the row-wise kernel (kokkos_mm_ThreadsTeams) over N, teams, threads, vec_len.
# Usage: just run this script from the mm-gpu-kokkos directory.

SIZES=(512 1024 2048 3072 4096 6144 8192 10000)
THREADS=(64 128 256)
VECS=(1 2 4)

OUT_FILE="rowwise_results.txt"
echo "Row-wise Teams sweep" > "$OUT_FILE"
echo "N teams threads vec_len time_s gflops" >> "$OUT_FILE"

export OMP_PROC_BIND=spread
export OMP_PLACES=threads

for N in "${SIZES[@]}"; do
  TEAMS_LIST=("$N" $((N/2)) $((N/4)))
  for teams in "${TEAMS_LIST[@]}"; do
    # avoid zero teams if N is small
    if (( teams <= 0 )); then continue; fi
    for threads in "${THREADS[@]}"; do
      for vec in "${VECS[@]}"; do
        # Respect the SYCL limit: team_size * vec_len <= 1024
        if (( threads * vec > 1024 )); then continue; fi
        echo "N=${N} teams=${teams} threads=${threads} vec=${vec}"
        output=$(./kokkos_mm_ThreadsTeams "$N" "$teams" "$threads" "$vec")
        time_s=$(printf "%s\n" "$output" | awk '/\[GPU\] Time:/ {print $3}')
        gflops=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3}')
        echo "${N} ${teams} ${threads} ${vec} ${time_s} ${gflops}" >> "$OUT_FILE"
      done
    done
  done
done

echo "Results written to ${OUT_FILE}"
