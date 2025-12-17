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
### row wise distribution
There are three parameters to tune: Number of Teams, Team size and Vector Length.
For Vector Length it is important that Team size x vector length must be smaler than 1024 (max work group size).
In our tests we found that vector length AUTO gave the best results.
To parallelize the workload we wnt with an row wise distribution of the resulting matrix to the teams.

For N=6144 the best configuration is 6114 Teams and 128 Threads per Team.
With this configuration every Team calculates a row of the resulting matrix. Every other combination of Teams and Threads per Team resulted in longer execution times.
With this configuration we achieved the following results:

``` 
[GPU] Time:       0.761619 s
[GPU] GFLOP/s:    609.04
[GPU] Checksum:   5.91474e+14
```
![](rowwise_perf.png)
Although this configuration dosnt alightn with the device properties of max work group size of 1024, it seems to be the optimal configuration for this problem size on this hardware. Simply due to overhead minimization by not splitting the work whilst still having enough threads to cover memory latency.

### tiling for N=6144
Top 10 configs by GFLOP/s ([for all measurements](../src/mm-gpu-kokkos/tiled_results_6144.txt)):
| tile_m | tile_n | team | vec | time_s | gflops  |
|-------:|-------:|-----:|----:|-------:|-------:|
| 64     | 16     | 64   | 16  | 0.428381 | 1082.81 |
| 64     | 32     | 64   | 16  | 0.429695 | 1079.50 |
| 128    | 16     | 128  | 8   | 0.432968 | 1071.34 |
| 128    | 8      | 128  | 8   | 0.441872 | 1049.75 |
| 64     | 16     | 64   | 4   | 0.450051 | 1030.67 |
| 128    | 8      | 64   | 4   | 0.451067 | 1028.35 |
| 64     | 8      | 64   | 4   | 0.451477 | 1027.42 |
| 64     | 8      | 64   | 8   | 0.453284 | 1023.32 |
| 128    | 16     | 64   | 4   | 0.456087 | 1017.04 |
| 84     | 8      | 84   | 8   | 0.456897 | 1015.23 |

#### Analysis
For the best four results team size * vector length = 1024 which is the max work group size of the device.
The best performances were achieved for rectangular tiles with tile_m > tile_n.
The tiled aproach
When runing those tiling configurations against N we got the following plot:
![](tiled_best_vs_n.png)
All configurations behave very similar and for N around 6144 they all achive over 1 TFLOP/s. 

## f)
Params for Row-wise: `N=6144`, `teams=6144`, `threads_per_team=128`, `vec_len=AUTO (-1)`
Params for Tiled: `N=6144`, `tile_m=64`, `tile_n=16`, `teams=64`, `vec_len=16`

| Layout | Row-wise GFLOP/s | Row-wise Time [s] | Tiled GFLOP/s | Tiled Time [s] |
| --- | ---: | ---: | ---: | ---: |
| RRR | **608.27** | 0.763 | 1079.94 | 0.429522 |
| RRL | ***648.45*** | 0.715 | 1087.55 | 0.426515 |
| RLR | 348.36 | 1.332 | ***1705.97*** | 0.271902 |
| RLL | 306.99 | 1.511 | ***1707.13*** | 0.271718 |
| LRR | 375.31 | 1.236 | 929.21 | 0.499195 |
| LRL | 351.24 | 1.321 | 932.62 | 0.497371 |
| LLR | 209.40 | 2.215 | 811.23 | 0.571792 |
| LLL | 210.37 | 2.205 | 816.12 | 0.568366 |

> The results are taken from the best of six consecutive runs for each configuration. No immense variance was observed between runs.

Notes:
In theory the best layout should be Matrix A row-major, Matrix B column-major and Matrix C row-major (RLR) to ensure best memory access for all three matrices during the innermost loop. For the tiled approach this is almost the case (RLL outperforms this by a tiny margin). However, for the row-wise approach RLR performs significantly worse than RRL and RRR. This could be due to cache effects or other hardware-specific optimizations that favor certain memory access patterns over others.

## g)
Using the theoreticaly best layout from f) (RLR) and the best configuration from e) we got

```
[GPU] Time:       0.273946 s
[GPU] GFLOP/s:    1693.24
[GPU] Checksum:   5.91474e+14
```
