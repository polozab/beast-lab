# Task 01

## a)

For compilation we used `icpx` version `2023.1.0` with the C++ toolchain provided by `GCC 14.2.0`.
Additionally to the specified modules in the README we had to load the latest GCC version and also explicitly used the latest module tags to always use the newest versions of the compiler and tools.
We also had to manually specify the GCC root so the newer `libstdc++` provided by the gcc module was linked instead of the old system installation under `/usr/lib64`.


## b)

For execution of the various executables we used the suggested OMP flags from Kokkos. As the environment implements OpenMP 4.5 these would be:

```sh
OMP_PROC_BIND=spread
OMP_PLACES=threads
```

## c)

When executing `kokkos_device_props_sycl` we get the following output:

```
Device Name: Intel(R) Data Center GPU Max 1550
Vendor: Intel(R) Corporation
Max Compute Units: 1024
Local Memory: 131072 bytes
Max Work Group Size: 1024
```

## d)

When running the matrix multiplication for `N=6144`, we observed the following execution times:

```
[CPU] Time:       32.9206 s
[CPU] GFLOP/s:    14.0902
[CPU] Checksum:   5.91474e+14
[GPU] Time:       1.75779 s
[GPU] GFLOP/s:    263.887
[GPU] Checksum:   5.91474e+14
``` 

### Bandwidth Calculation

Data transferred = 3 * N * N * 8 = 0.906 GB

|        | GPU     | CPU     |
|--------|---------|---------|
| Time (s) | 1.75779 | 32.9206 |
| Bandwidth {Data Transferred / Time}(GB/s) | 0.515   | 0.0275 |

