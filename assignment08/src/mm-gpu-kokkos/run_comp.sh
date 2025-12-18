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

echo "size,handwritten_min_gflops,handwritten_max_gflops,handwritten_avg_gflops,kernel_min_gflops,kernel_max_gflops,kernel_avg_gflops,handwritten_min_time_s,handwritten_max_time_s,handwritten_avg_time_s,kernel_min_time_s,kernel_max_time_s,kernel_avg_time_s"

for n in $(seq "$start" "$step" "$end"); do

    handwritten_vals=()
    kernel_vals=()
    handwritten_time_vals=()
    kernel_time_vals=()

    for i in $(seq 1 "$runs"); do

        echo "Size $n run $i/$runs" >&2
        output=$(OMP_PROC_BIND=spread OMP_PLACES=threads "./kokkos_kernel_mm_comp" "$n")
        echo $output >&2

        handwritten=$(printf "%s\n" "$output" | awk '/\[Handwritten\] GFLOP\/s:/ {print $3; exit}')
        kernel=$(printf "%s\n" "$output" | awk '/\[Kernel\] GFLOP\/s:/ {print $3; exit}')
        handwritten_time=$(printf "%s\n" "$output" | awk '/\[Handwritten\] Time:/ {print $3; exit}')
        kernel_time=$(printf "%s\n" "$output" | awk '/\[Kernel\] Time:/ {print $3; exit}')

        if [[ -z "$handwritten" || -z "$kernel" || -z "$handwritten_time" || -z "$kernel_time" ]]; then
            echo "failed to parse GFLOP/s or Time for size $n run $i" >&2
            exit 1
        fi

        handwritten_vals+=("$handwritten")
        kernel_vals+=("$kernel")
        handwritten_time_vals+=("$handwritten_time")
        kernel_time_vals+=("$kernel_time")

    done

    handwritten_stats=$(printf "%s\n" "${handwritten_vals[@]}" | calc_stats)
    kernel_stats=$(printf "%s\n" "${kernel_vals[@]}" | calc_stats)
    handwritten_time_stats=$(printf "%s\n" "${handwritten_time_vals[@]}" | calc_stats)
    kernel_time_stats=$(printf "%s\n" "${kernel_time_vals[@]}" | calc_stats)
    echo "$n,$handwritten_stats,$kernel_stats,$handwritten_time_stats,$kernel_time_stats"

done
