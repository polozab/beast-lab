# Task 2 - Measurement with Likwid

## b)

## c)

For our measurements we used the newest available version of gcc and the same compiler flags

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

In total our software-measured results were very accurate and reflect the results measured by likwid on all systems.

### ii)

When looking at the percentage of AVX instruction used, we see 100% usage of vectorized instruction. The same also appies to the ARM based systems with ...

### iii) **TODO**

### iv)