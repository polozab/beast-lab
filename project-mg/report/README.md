# Report on the Project optimizing a Multigrid Kernel 

## Project Setup and Building

To start optimization of the provided kernel we first had to understand the basic structure and methodology of the code. We also started searching for general optimization guides of the given Grace Hopper architecture to understand the available toolset. We quickly found several resources which provided good instruction on how to tune programs for the Grace Hopper CPUs. We though initially it would be safest to follow the [official tuning guide](https://docs.nvidia.com/dccpu/grace-perf-tuning-guide/index.html) provided by NVIDIA. It gave us a great overview over the system specifics and showed entry points to different development tools.

### Compiler

The first decision we had to make was which compiler to use for the compilation of our multigrid executable. As previous assignments showed that vendor specific compiler were in almost all cases the better choice when it comes to performance we chose the NVIDIA HPC compiler for C++ `nvc++`, which comes preinstalled on the Grace Hopper system via the NVIDIA HPC SDK. As other compilers often target for portability, which we don't need in our case, according to NVIDIA [(*)](https://docs.nvidia.com/dccpu/grace-perf-tuning-guide/compilers.html#nvidia-hpc-compilers) the NVHPC compiler directly targets system specific optimizations. This principle in addition to the fact that both compiler and architecture were build by NVIDIA simplifies the building process as `nvc++` automatically optimizes for the Grace Hopper architecture for recent versions.

To access the NVIDIA HPC SDK and the `nvc++` compiler we can load the module with `module load nvhpc` which automatically fetches version `25.3`:

```
$nvc++ --version
nvc++ 25.3-0 linuxarm64 target on aarch64 Linux -tp grace 
NVIDIA Compilers and Tools
Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES.  All rights reserved.

```

As we see it automatically picks up that our target platform will be `grace`.

### Compiler setting and flags



Added several compiler settings according to [NVIDIA developer manual](https://docs.nvidia.com/hpc-sdk/compilers/hpc-compilers-user-guide/index.html#frequently-used-options)
