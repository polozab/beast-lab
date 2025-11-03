# Task 2 - Measurement with Likwid

## b)

## c)

### sap3 **TODO**: Fix "./data/L3_T1_L3"

**L1 Cache**: 48KB / 4 / 8 = 1,5KB = 1,46 KiB = 2^1 * 2^10 B = 2^11 B ==> N = 10
**L2 Cache**: 2048 KB / 4 / 8 = 64KB = 62,5 KiB = 2^7 * 2^10 B = 2^16 B ==> N = 15
**L3 Cache**: 105 MB / 4 / 8 = 3,28MB = 3,13 MiB = 2^2 * 2^20 B = 2^22 B ==> N = 21

**L1 Cache (Multi)**: Unchanged as local ==> N = 10
**L2 Cache (Multi)**: Unchanged as local ==> N = 15
**L3 Cache (Multi)**: 105 MB / 4 / 8 / 48 = 68 KB = 66,41 KiB = 2^7 * 2^10 B = 2^17 B ==> N = 16

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

### gh2

### cs2

## d)

### i)

In total our software-measured results were very accurate and reflect the results measured by likwid on all systems.

### ii)

When looking at the percentage of AVX instruction used, we see 100% usage of vectorized instruction. The same also appies to the ARM based systems with ...

### iii) **TODO**

### iv)