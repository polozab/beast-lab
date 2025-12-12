#!/usr/bin/bash

module load gcc/14.2.0-gcc-13.2.0-57z6vik
module load compiler/latest

export KOKKOS_ROOT=~/kokkos_intel/install
export KOKKOS_SRC=~/kokkos_install

make