# Task 2 - Measurement with Likwid

## b)

## c)

### sap3

**L1 Cache**: 48 KB / 4 / 8 = 1.5 KB = 1.46 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 2048 KB / 4 / 8 = 64 KB = 62.5 KiB = 2^7 * 2^10 B = 2^17 B ==> N = 16
**L3 Cache**: 105 MB / 4 / 8 = 3.281 MB = 3.129 MiB = 2^2 * 2^20 B = 2^22 B ==> N = 21

**L1 Cache (Multi)**: Unchanged as local ==> N = 10
**L2 Cache (Multi)**: Unchanged as local ==> N = 16
**L3 Cache (Multi)**: 105MB / 4 / 8 / 48 = 0.068359 MB = 68.359 KB = 66.41 KiB = 2^7 * 2^10 B = 2^17 B ==> N = 16

| Size     | #Threads | FLOPS (MFLOP/s) | Memory BW (MB/s) | L3-L2 BW    | L2-L1 BW    |
|----------|----------|-----------------|------------------|-------------|-------------|
| < L1     | 1        | 18778.6216      | 42.7161          | 0.1072      | 1.8969      |
| > L1     | 1        | 8105.6861       | 42.5999          | 1.9063      | 132168.4850 |
| > L2     | 1        | 1608.0864       | 695.0834         | 44877.8896  | 26358.3217  |
| > L3     | 1        | 1125.1157       | 17955.4741       | 36030.2552  | 17913.0157  |
| < SUM L1 | 96       | 49554.9104      | 1510.0123        | 20258.0872  | 19366.2828  |
| > SUM L1 | 96       | 241922.0931     | 1993.7671        | 21888.4559  | 18019.2513  |
| > SUM L2 | 96       | 352750.5634     | 302.7993         | 18548.5551  | 6389990     |
| > SUM L3 | 96       | 4373.4741       | 124471.5185      | 143195.4787 | 73567.4410  |

### milan2

**L1 Cache**: 32 KB / 4 / 8 = 1 KB = 0.977 KiB = 2^0 * 2^10 B = 2^10 B ==> N = 9
**L2 Cache**: 512 KB / 4 / 8 = 16 KB = 15.625 KiB = 2^4 * 2^10 B = 2^14 B ==> N = 13
**L3 Cache**: 96 MB / 4 / 8 = 3 MB = 2.861 MiB = 2^1 * 2^20 B = 2^21 B ==> N = 20

**L1 Cache (Multi)**: Unchanged as local ==> N = 9
**L2 Cache (Multi)**: Unchanged as local ==> N = 13
**L3 Cache (Multi)**: 96 MB / 4 / 8 / 8 = 0.375 MB = 375 KB = 366.211 KiB = 2^8 * 2^10 B = 2^18 B ==> N = 18

### gh3

**L1 Cache**: 64 KB / 4 / 8 = 2 KB = 1.953 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 1024 KB / 4 / 8 = 32 KB = 31.25 KiB = 2^5 * 2^10 B = 2^15 B ==> N = 14
**L3 Cache**: 114 MB / 4 / 8 = 3.5625 MB = 3.397 MiB = 2^2 * 2^20 B = 2^22 B ==> N = 21

**L1 Cache (Multi)**: Unchanged as local ==> N = 10
**L2 Cache (Multi)**: Unchanged as local ==> N = 14
**L3 Cache (Multi)**: 114 MB / 4 / 8 / 72 = 0.049479 MB = 49.479 KB = 48 KiB = 2^5 * 2^10 B = 2^15 B ==> N = 15

### cs2

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