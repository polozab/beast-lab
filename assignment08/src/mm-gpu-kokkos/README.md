# Overview
In this README, we provide basic hints for building Kokkos on various beast systems targeting both the CPUs and GPUS. We provide separate makefiles for each system (Intel, AMD, and Nvidia).
Please take these (and the provided Makefiles) as hints, since we have not systematically tested the, but we trust you can use them for this assignment and resolve the minor bugs/issues that might appear when using them. We also appreciate your feedback on these instructions.

## Intel systems

### Clone Kokkos sources
```bash
git clone -b 4.6.01 https://github.com/kokkos/kokkos.git kokkos_intel
cd kokkos_intel
mkdir build
cd build
```

### Setup the environment
```bash
module load gcc/14.2.0-gcc-13.2.0-57z6vik
module load tbb/latest
module load compiler-rt/latest
module load cmake
module load compiler/latest
module load dpl/latest
module load vtune/latest

export KOKKOS_ROOT=<your home directory>/kokkos_intel/install/
export KOKKOS_SRC=<your home directory>/kokkos_install/
```

### Building Kokkos
```bash
cmake    -DCMAKE_CXX_COMPILER=icpx    -DKokkos_ENABLE_SERIAL=ON    -DKokkos_ENABLE_OPENMP=ON   -DCMAKE_CXX_STANDARD=20 -DKokkos_ENABLE_SYCL=ON    -DKokkos_ARCH_INTEL_PVC=ON    -DCMAKE_CXX_FLAGS="-fp-model=precise" -DKokkos_ENABLE_LIBDL=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ..

make -j
make install
```

### Build the assignment codes
```bash
cd assignment05/src/mm-gpu-kokkos # Alter aus welchem Jahr ist die Datei (┛◉Д◉)┛彡┻━┻
make -f Makefile.intel
```

### Simple test run of the assignment codes
```bash
make -f Makefile.intel run_all
```

## Nvidia systems
### Clone Kokkos sources
```bash
git clone -b develop  https://github.com/kokkos/kokkos.git kokkos_nvidia
cd kokkos_nvidia
mkdir build
cd build
```
### Setup the environment
```bash
module load cmake
module load nvhpc
export KOKKOS_ROOT=<your home directory>/kokkos_nvidia/install/
export KOKKOS_SRC=<your home directory>/kokkos_nvidia/
export KOKKOS_PATH=$KOKKOS_SRC
export CUDA_ROOT=${NVHPC_ROOT}/cuda/
```

### Building Kokkos
```bash
cmake -DCMAKE_CXX_COMPILER=${KOKKOS_SRC}/bin/nvcc_wrapper -D CMAKE_VERBOSE_MAKEFILE=ON  -DKokkos_ENABLE_SERIAL=ON    -DCMAKE_CXX_STANDARD=20   -DKokkos_ENABLE_OPENMP=ON CMAKE_CXX_FLAGS='-Werror=all-warnings -Werror' -DKokkos_ARCH_HOPPER90=ON -DKokkos_ENABLE_CUDA=ON -DKokkos_ENABLE_COMPILER_WARNINGS=ON -DKokkos_ENABLE_LIBDL=ON -DKokkos_ENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ..

make -j
make install
```

### Build the assignment codes
```bash
cd assignment05/src/mm-gpu-kokkos
make -f Makefile.nvidia
```

### Simple test run of the assignment codes
```bash
make -f Makefile.nvidia run_all
```

## AMD systems
### Clone Kokkos sources
```bash
git clone -b develop  https://github.com/kokkos/kokkos.git kokkos_amd
cd kokkos_amd
mkdir build
cd build
```

### Setup the environment
```bash
module load cmake

export ROCM_PATH=/opt/rocm/
export KOKKOS_ROOT=<your home directory>/kokkos_amd/install/
export KOKKOS_SRC=<your home directory>/kokkos_amd/
```
### Building Kokkos
```bash
cmake -DCMAKE_CXX_COMPILER=hipcc -DKokkos_ENABLE_SERIAL=ON -DKokkos_ENABLE_OPENMP=ON -DCMAKE_CXX_STANDARD=20 -DKokkos_ENABLE_HIP=ON -DKokkos_ENABLE_COMPILER_WARNINGS=ON -DKokkos_ENABLE_TESTS=ON -DKokkos_ENABLE_LIBDL=ON -DKokkos_ARCH_AMD_GFX90A=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install ..

make -j
make install
```

### Build the assignment codes
```bash
cd assignment05/src/mm-gpu-kokkos
make -f Makefile.amd
```

### Simple test run of the assignment codes
```bash
make -f Makefile.amd run_all
```