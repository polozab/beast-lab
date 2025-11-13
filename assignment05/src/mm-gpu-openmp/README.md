##  MxM OpenMP GPU Offloading

### Setup environment on the NVIDIA GPU machines
        
* Load the modules: `module load nvhpc/25.3`
* Minimal flags to enable offloading at compile time: `-fopenmp -mp=gpu`

### Setup environment on the AMD GPU machines
* The default environment on the AMD machines is already setup.
* Minimal flags to enable offloading at compile time: `clang -fopenmp -target x86 64-pc-linux-gnu -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx90a <your code file>`
	
### Setup environment on the Intel GPU machines
* Load the modules: module laod umf tbb compiler-rt compiler
* Minimal flags to enable offloading at compile time: `-fsycl -fiopenmp -fopenmp-targets=spir64`
