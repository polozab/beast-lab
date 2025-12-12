#!/usr/bin/bash

module load tbb/latest
module load compiler-rt/latest
module load compiler/latest
module load dpl/latest
module load vtune/latest

export KOKKOS_ROOT=~/kokkos_intel/install
export KOKKOS_SRC=~/kokkos_install

make