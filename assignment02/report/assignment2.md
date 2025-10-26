
# Task 2 - Experiments and Measurements

## a)

After testing different compilers and compiler flags the fastest performance could be archived by far with the g++ (Spack GCC) 13.1.0 compiler. In combination with the following flags we could reach a peak performance of around 2612 MFLOPs (N=100, unoptimized and sequential).

~~~
g++ -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp -o assignment assignment.out
~~~

The same performance was gained when just complied with.

~~~
g++ -Ofast -march=native -fopenmp -o assignment assignment.out
~~~


The other clang c compiler was significantly slower. Also notably leaving out certain command arguments had an huge impact on performance. Especially *-ffast-math* which improved our performance from 399 to 2612 MFLOPs

## b) **TODO**

When taking a look at the disassembled object dump, you can the spot FADDA SVE vector instruction being used in the most inner loop.

~~~
  400e18:       a5e042a0        ld1d    {z0.d}, p0/z, [x21, x0, lsl #3]   ## load next vector
  400e1c:       04f0e3e0        incd    x0                ## Increase k
  400e20:       65d82001        fadda   d1, p0, d1, z0.d  ## Perform partial sum
  400e24:       25f40c00        whilelo p0.d, w0, w20     ## Checking loop cond
  400e28:       54ffff81        b.ne    400e18            ## Branching on loop cond
~~~

This instruction calculates the partial sum by taking 

## c)

|       | Order   | MFLOPs   |
|-------|---------|----------|
| **1** | **ikj** | **5140** |
| 2     | kij     | 4587     |
| 3     | jik     | 2796     |
| **4** | **ijk** | **2628** |
| 5     | jki     | 649      |
| 6     | kji     | 194      |

**TODO:**
- Huge performance improvement (doubling)
- C, B access continuos
- A static for inner loop (no column jumps necessary)
- In assembly more extensive use of SVE (with code example)

## d)