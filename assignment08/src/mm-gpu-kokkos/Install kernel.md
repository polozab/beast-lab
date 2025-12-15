

## Intel systems

### Clone Kokkos sources
```bash
git clone -b 4.6.01 https://github.com/kokkos/kokkos-kernels.git kokkos_kernel_intel
cd kokkos_kernel_intel
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

export KOKKOS_KERNEL_ROOT=~/kokkos_kernel_intel/install/
export KOKKOS_KERNEL_SRC=~/kokkos_kernel_install/
export KOKKOS_ROOT=~/kokkos_intel
export KOKKOS_SRC=~/kokkos_intel/install
```

### Building Kokkos
```bash
cmake    -DCMAKE_CXX_COMPILER=icpx   -DCMAKE_CXX_STANDARD=20  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../install -DBUILD_SHARED_LIBS=ON -DKokkos_ROOT=${KOKKOS_SRC} -DKokkos_DIR=${KOKKOS_ROOT} ..

make -j
make install
```
