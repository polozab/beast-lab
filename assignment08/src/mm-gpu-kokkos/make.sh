#!/usr/bin/bash

module load gcc/latest
module load compiler/latest

export KOKKOS_ROOT=~/kokkos_intel/install
export KOKKOS_SRC=~/kokkos_install

make