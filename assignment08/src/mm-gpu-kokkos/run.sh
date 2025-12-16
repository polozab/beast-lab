#!/usr/bin/env bash
set -euo pipefail

export GCC_ROOT=/home/sw/spack_new/spack_stack_dev/sap/spack/opt/spack/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder_/linux-sles15-skylake_avx512/gcc-7.5.0/gcc-13.2.0-kv6jgtumk27vhunessefvgbwq5twgtom
export KOKKOS_ROOT=~/kokkos_intel/install
export KOKKOS_KERNEL_ROOT=~/kokkos_kernel_intel/install
export LD_LIBRARY_PATH=/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/oclfpga/host/linux64/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/x64:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/compiler/lib/intel64_lin:/opt/software/x86/oneAPI/tbb/2021.9.0/lib/intel64/gcc4.8:$GCC_ROOT/lib64:$KOKKOS_ROOT/lib64:$KOKKOS_KERNEL_ROOT/lib64

runs=${1:-5}
start=${2:-200}
end=${3:-10000}
step=${4:-200}

calc_stats() {
    awk '
        NR==1 {min=max=$1}
        {if($1<min) min=$1; if($1>max) max=$1; sum+=$1; n++}
        END {
            if (n==0) {print "0,0,0"; exit}
            printf "%.6f,%.6f,%.6f\n", min, max, sum/n
        }
    '
}

echo "size,cpu_min_gflops,cpu_max_gflops,cpu_avg_gflops,gpu_min_gflops,gpu_max_gflops,gpu_avg_gflops"

for n in $(seq "$start" "$step" "$end"); do
    cpu_vals=()
    gpu_vals=()

    for i in $(seq 1 "$runs"); do
        echo "Size $n run $i/$runs" >&2
        output=$(OMP_PROC_BIND=spread OMP_PLACES=threads "./kokkos_kernel_mm" "$n")

        cpu=$(printf "%s\n" "$output" | awk '/\[CPU\] GFLOP\/s:/ {print $3; exit}')
        gpu=$(printf "%s\n" "$output" | awk '/\[GPU\] GFLOP\/s:/ {print $3; exit}')

        if [[ -z "$cpu" || -z "$gpu" ]]; then
            echo "failed to parse GFLOP/s for size $n run $i" >&2
            exit 1
        fi

        cpu_vals+=("$cpu")
        gpu_vals+=("$gpu")
    done

    cpu_stats=$(printf "%s\n" "${cpu_vals[@]}" | calc_stats)
    gpu_stats=$(printf "%s\n" "${gpu_vals[@]}" | calc_stats)
    echo "$n,$cpu_stats,$gpu_stats"
done
