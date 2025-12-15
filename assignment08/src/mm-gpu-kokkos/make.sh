#!/usr/bin/bash

module load gcc/14.2.0-gcc-13.2.0-57z6vik
module load compiler/latest

export GCC_ROOT=/home/sw/spack_new/spack_stack_dev/sap/spack/opt/spack/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder__/__spack_path_placeholder_/linux-sles15-icelake/gcc-13.2.0/gcc-14.2.0-57z6vikjlbeafvcvfuq5u65hgsjsi5bk

export KOKKOS_ROOT=~/kokkos_intel/install

export KOKKOS_KERNEL_ROOT=~/kokkos_kernel_intel/install

export LD_LIBRARY_PATH=/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/oclfpga/host/linux64/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/lib/x64:/opt/software/x86/oneAPI/compiler/2023.1.0/linux/compiler/lib/intel64_lin:/opt/software/x86/oneAPI/tbb/2021.9.0/lib/intel64/gcc4.8:$GCC_ROOT:$KOKKOS_ROOT:$KOKKOS_KERNEL_ROOT

make