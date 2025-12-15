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

## e) 
For N=6144 the best configuration is 6114 Teams and 128 Threads per Team.
With this configuration every Team calculates a row of the resulting matrix. Every other combination of Teams and Threads per Team resulted in longer execution times.
With this configuration we achieved the following results:

``` 
[GPU] Time:       0.761619 s
[GPU] GFLOP/s:    609.04
[GPU] Checksum:   5.91474e+14
```

Although this configuration dosnt alightn with the device properties of max work group size of 1024, it seems to be the optimal configuration for this problem size on this hardware. Simply due to overhead minimization by not splitting the work whilst still having enough threads to cover memory latency.

## f)
Params: `N=6144`, `teams=6144`, `threads_per_team=128`, `vec_len=AUTO (-1)`

| Layout (A,B,C) | GFLOP/s | Time [s] |
| --- | ---: | ---: |
| RRL | ***648.45*** | 0.715 |
| RRR | **608.27** | 0.763 |
| LRR | 375.31 | 1.236 |
| LRL | 351.24 | 1.321 |
| RLR | 348.36 | 1.332 |
| RLL | 306.99 | 1.511 |
| LLL | 210.37 | 2.205 |
| LLR | 209.40 | 2.215 |

> The results are taken from the best of six consecutive runs for each configuration. No immense variance was observed between runs.

Notes:
In theory the best layout should be Matrix A row-major, Matrix B column-major and Matrix C row-major (RLR) to ensure contiguous memory access for all three matrices during the innermost loop.

## g)
Using the best layout from f) (RRL) and the best configuration from e) (6144 teams, 128 threads per team) we got

```
[GPU] Time:       0.715487 s
[GPU] GFLOP/s:    648.309
[GPU] Checksum:   5.91474e+14
```
So no?