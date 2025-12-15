#!/usr/bin/env bash
set -euo pipefail

runs=${1:-10}
size=${2:-4096}

cpu_vals=()
gpu_vals=()

for i in $(seq 1 "$runs"); do
    echo "Run $i/$runs"
    output=$(OMP_PROC_BIND=spread OMP_PLACES=threads ./kokkos_mm "$size")
    echo "$output"

    cpu_gflops=$(printf "%s\n" "$output" | awk '/\\[CPU\\] GFLOP\\/s:/ {print $3}')
    gpu_gflops=$(printf "%s\n" "$output" | awk '/\\[GPU\\] GFLOP\\/s:/ {print $3}')

    cpu_vals+=("$cpu_gflops")
    gpu_vals+=("$gpu_gflops")
done

stats() {
    printf "%s\n" "$@" | awk '
        NR==1 {min=max=$1}
        {if($1<min) min=$1; if($1>max) max=$1; sum+=$1; n++}
        END {printf "min=%.5f max=%.5f avg=%.5f\n", min, max, sum/n}
    '
}

echo "CPU GFLOP/s stats:"
printf "%s\n" "${cpu_vals[@]}" | stats

echo "GPU GFLOP/s stats:"
printf "%s\n" "${gpu_vals[@]}" | stats
