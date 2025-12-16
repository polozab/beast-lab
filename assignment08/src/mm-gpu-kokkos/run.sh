#!/usr/bin/env bash

export GCC_ROOT=/home/sw/spack_new/spack_stack_dev/sap/spack/opt/spack/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder_/linux-sles15-skylake_avx512/gcc-7.5.0/gcc-13.2.0-kv6jgtumk27vhunessefvgbwq5twgtom
export KOKKOS_ROOT=~/kokkos_intel/install
export KOKKOS_KERNEL_ROOT=~/kokkos_kernel_intel/install
export LD_LIBRARY_PATH=/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/oclfpga/host/linux64/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/x64:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/compiler/lib/intel64_lin:/opt/software/x86/oneAPI/tbb/2021.9.0/lib/intel64/gcc4.8:$GCC_ROOT/lib64:$KOKKOS_ROOT/lib64:$KOKKOS_KERNEL_ROOT/lib64

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
