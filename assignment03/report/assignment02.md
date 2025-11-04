# Task 2 - Measurement with Likwid

## b)

In the following we will take a closer look at calculations done by likwid for the *SAP3* system.

### FLOPS/s

The FLOPS/s calculation is done using the following formular:

~~~
DP [MFLOP/s] = 1.0E-06*(FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE*2+FP_ARITH_INST_RETIRED_SCALAR_DOUBLE+FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE*4+FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE*8)/runtime
~~~

First we scale the result to MFLOPS by dividing with 10^6. The actual calculation is based on the finished (or retired) double FLOPS. For that we take the scalar and vectorized instructions and scale them according to how many doubles get processed with each. E.g. a scaler 64 bit double FLOP doesn't get scaled, while a 512 bit double FLOP gets scaled by 8. Thus we get all processed double FLOP which we then divide by the runtime of the measure region.

The calculation uses the following events:

  - FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE: finished 512 bit AVX vector double FLOPs
  - FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE: finished 256 bit AVX vector double FLOPs
  - FP_ARITH_INST_RETIRED_128B_PACKED_DOUBLE: finished 128 bit AVX vector double FLOPs
  - FP_ARITH_INST_RETIRED_SCALAR_DOUBLE: finished scalar double FLOPs

### Memory Bandwidth

To calculate the (Main-)Memory bandwidth likwid uses the following formular:

~~~
Memory bandwidth [MBytes/s] = 1.0E-06*(SUM(CAS_COUNT_RD)+SUM(CAS_COUNT_WR))*64.0/runtime
~~~

First of all we again scale the result to Megabytes/s. After that the memory bandwidth gets calculated by simply summing the count of read and write operations sent to DRAM over every memory controller. At last we again divide by the runtime ov the measure region.

The calculation uses the following events:

  - CAS_COUNT_RD: count of read column address strobe signals
  - CAS_COUNT_WR: count of write column address strobe signals
  
The column address strobe specifies which column contains the data needed after a row has been selected and is send by the processor to the DRAM chip.
An important notice by likwid is that this event is only possible to measure on a per-socket level.

### L1 <-> L2 Bandwidth

To calculate the (load) bandwidth between the L1D and L2 cache the following formular is used:

~~~
L2D (load) bandwidth [MBytes/s] = 1.0E-06*L1D_REPLACEMENT*64.0/time
~~~

First we scale the result to Megabytes/s. The bandwidth is calculated by taking the number of cache line replacements in the L1D cache, which gets multiplied by the size of a L1D cache line (64 bytes). Finally we again divide this by the runtime of the region.
However this only accounts for the read operations done when the L1 cache loads data from the L2 cache. Thus write backs from L1 to L2 which would also contribute to the memory bandwidth get ignored. likwid notices that there is currently no formular for calculating the number of write backs or the associated bandwidth needed for these write operations. We are therefor currently limited to only measure the read operations when calculating the bandwidth between L1D and L2 caches.

It uses the L1D_REPLACEMENT event for measuring the number of cache line allocated or replaced during execution.

### L2 <-> L3 Bandwidth

Calculating the bandwidth between the L2 and L3 cache is done by using th following formular:

~~~
L3 bandwidth [MBytes/s] = 1.0E-06*(L2_LINES_IN_ALL+L2_LINES_OUT_NON_SILENT)*64/time
~~~

For the last time we first scale the result to Megabytes/s. To calculate the actual bandwidth we add up the number of allocated cache lines in the L2 cache and the number of modified cache lines evicted from the L2 cache. This number we, again, multiply by the length of one cache line (64 bytes). Finally we divide this by the runtime of the region to get the cache bandwidth used between L2 and L3 caches.

The calculation is based on the following events:

  - L2_LINES_IN_ALL: Number of allocated L2 cache lines
  - L2_LINES_OUT_NON_SILENT: Number of modified cache lines evicted from L2 cache

An important notice by likwid is that this also includes transfers which were caused by a write allocate load on a L2 cache miss.

## c)

For our measurements we used the newest available version of gcc and the same compiler flags for all systems.
First we tried to calculate an appropriate vector size to fit all necessary data into a specific cache level.
For that we followed a basic calculation:
~~~
CACHE_SIZE / 4 vectors to fit / 8 bytes per double >= vector size (number of elements)
                                                   >= 2^N
~~~


### sap3

~~~
Compiler: gcc version 14.2.0 (Spack GCC)
Flags:    gcc -fopenmp -O3 -ffast-math -march=native
~~~

**L1 Cache**: 48 KB / 4 / 8 = 1.5 KB = 1.46 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 2048 KB / 4 / 8 = 64 KB = 62.5 KiB = 2^7 * 2^10 B = 2^17 B ==> N = 16
**L3 Cache**: 105 MB / 4 / 8 = 3.281 MB = 3.129 MiB = 2^2 * 2^20 B = 2^22 B ==> N = 21

**L1 Cache (Multi)**: Unchanged as local ==> N = 10
**L2 Cache (Multi)**: Unchanged as local ==> N = 16
**L3 Cache (Multi)**: 105MB / 4 / 8 / 48 = 0.068359 MB = 68.359 KB = 66.41 KiB = 2^7 * 2^10 B = 2^17 B ==> N = 16

| Size     | #Threads | FLOPS (MFLOP/s) | Memory BW (MB/s) | L3-L2 BW    | L2-L1 BW   |
|----------|----------|-----------------|------------------|-------------|------------|
| < L1     | 1        | 18792.3371      | 42.8359          | 0.1025      | 1.8873     |
| > L1     | 1        | 3399.4238       | 44.2688          | 41331.4623  | 56533.6299 |
| > L2     | 1        | 1663.8956       | 441.9987         | 52951.3745  | 26195.2111 |
| > L3     | 1        | 1118.2067       | 17934.8847       | 36409.5351  | 18033.5804 |
| < SUM L1 | 96       | 52789.1321      | 1631.0587        | 20777.0380  | 18199.0698 |
| > SUM L1 | 96       | 341590.7250     | 1682.6038        | 18031.5534  | 20668.5580 |
| > SUM L2 | 96       | 350588.4812     | 1452.4051        | 17499.1981  | 19913.4245 |
| > SUM L3 | 96       | 5006.7430       | 125258.1201      | 152621.7798 | 78029.0008 |

### milan2

~~~
Compiler: gcc version 14.2.0 (Spack GCC)
Flags:    gcc -fopenmp -O3 -ffast-math -march=native
~~~

**L1 Cache**: 32 KB / 4 / 8 = 1 KB = 0.977 KiB = 2^0 * 2^10 B = 2^10 B ==> N = 9
**L2 Cache**: 512 KB / 4 / 8 = 16 KB = 15.625 KiB = 2^4 * 2^10 B = 2^14 B ==> N = 13
**L3 Cache**: 96 MB / 4 / 8 = 3 MB = 2.861 MiB = 2^1 * 2^20 B = 2^21 B ==> N = 20

**L1 Cache (Multi)**: Unchanged as local ==> N = 9
**L2 Cache (Multi)**: Unchanged as local ==> N = 13
**L3 Cache (Multi)**: 96 MB / 4 / 8 / 8 = 0.375 MB = 375 KB = 366.211 KiB = 2^8 * 2^10 B = 2^18 B ==> N = 18

| Size     | #Threads | FLOPS (MFLOP/s) | Memory BW (MB/s) | L3-L2 BW    | L2-L1 BW     |
|----------|----------|-----------------|------------------|-------------|--------------|
| < L1     | 1        | 17338.0800      | 212.3589         | 0.0930      | 1.3994       |
| > L1     | 1        | 7009.0163       | 217.889          | 159.4738    | 112160.0700  |
| > L2     | 1        | 5087.9138       | 328.9438         | 80199.9826  | 81524.8608   |
| > L3     | 1        | 1837.5122       | 37193.7068       | 20852.0219  | 29667.2002   |
| < SUM L1 | 96       | 876686.4024     | 455.5005         | 42094.4838  | 34971.2595   |
| > SUM L1 | 96       | 1.387370e+06    | 459.1792         | 29.0984     | 44.3742      |
| > SUM L2 | 96       | 613255.2799     | 9025.0515        | 56896.0060  | 9.278310e+06 |
| > SUM L3 | 96       | 8297.3891       | 118454.5689      | 179082.2196 | 89936.9129   |

### gh3

~~~
Compiler: gcc version 14.2.0 (Spack GCC)
Flags:    gcc -fopenmp -O3 -ffast-math -march=native
~~~

**L1 Cache**: 64 KB / 4 / 8 = 2 KB = 1.953 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 1024 KB / 4 / 8 = 32 KB = 31.25 KiB = 2^5 * 2^10 B = 2^15 B ==> N = 14
**L3 Cache**: 114 MB / 4 / 8 = 3.5625 MB = 3.397 MiB = 2^2 * 2^20 B = 2^22 B ==> N = 21

**L1 Cache (Multi)**: Unchanged as local ==> N = 10
**L2 Cache (Multi)**: Unchanged as local ==> N = 14
**L3 Cache (Multi)**: 114 MB / 4 / 8 / 72 = 0.049479 MB = 49.479 KB = 48 KiB = 2^5 * 2^10 B = 2^15 B ==> N = 15

| Size     | #Threads | FLOPS (MFLOP/s) | Memory BW (MB/s) | L3-L2 BW    | L2-L1 BW    |
|----------|----------|-----------------|------------------|-------------|-------------|
| < L1     | 1        | 9808.7074       | 791.6614         | 1.4163      | 1.4933      |
| > L1     | 1        | 7112.0756       | 787.2121         | 4.4922      | 85315.1576  |
| > L2     | 1        | 3964.7740       | 2255.9094        | 256316.4212 | 165126.4142 |
| > L3     | 1        | 1617.5252       | 27666.5331       | 37972.4716  | 64715.6954  |
| < SUM L1 | 96       | 318142.3843     | 587.4749         | 206692.0253 | 239873.0295 |
| > SUM L1 | 96       | 279205.9027     | 565.1667         | 101415.3190 | 137930.8226 |
| > SUM L2 | 96       | 305186.4285     | 557.3346         | 93848.2197  | 132536.8048 |
| > SUM L3 | 96       | 14441.6739      | 236410.6024      | 292269.5583 | 592789.1713 |

### cs2

~~~
Compiler: gcc version 13.1.0 (Spack GCC)
Flags:    gcc -fopenmp -O3 -ffast-math -march=native
~~~

**L1 Cache**: 64 KiB / 4 / 8 = 2 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 8 MiB / 4 / 8 = 0.25 MiB = 256 KiB = 2^8 * 2^10 B = 2^18 B ==> N = 17
**L3 Cache**: No L3 cache

**L1 Cache (Multi)**: Unchanged as local ==> N = 9
**L2 Cache (Multi)**: 8 MiB / 4 / 8 / 12 = 21.2992 KiB = 2^5 * 2^10 B = 2^15 B ==> N = 15
**L3 Cache (Multi)**: No L3 cache

## d)

### i)

In total our software-measured results for FLOPs/s were very accurate and reflect the results measured by likwid on all tested systems.

### ii)

When looking at the percentage of AVX instruction used, we see 100% usage of vectorized instruction. The same also appies to the ARM based systems with ...

### iii) **TODO**

### iv)