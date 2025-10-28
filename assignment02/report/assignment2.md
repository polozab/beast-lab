
# Task 2 - Experiments and Measurements

## a)

After testing different compilers and compiler flags the fastest performance could be archived by far with the g++ (Spack GCC) 13.1.0 compiler.
In combination with the following flags we could reach a peak performance of around 2612 MFLOPs (N=100, unoptimized and sequential).

~~~
g++ -O3 -march=native -ffast-math -fstrict-aliasing -fopenmp -o assignment assignment.out
~~~

The same performance was gained when just complied with.

~~~
g++ -Ofast -march=native -fopenmp -o assignment assignment.out
~~~

The other clang c compiler was significantly slower. Also notably leaving out certain command arguments had an huge impact on performance.
Especially *-ffast-math* which improved our performance from 399 to 2612 MFLOPs

## b)

When taking a look at the disassembled object dump, you can the spot *FADDA* SVE vector instruction being used in the most inner loop.

~~~
  400e18:       a5e042a0        ld1d    {z0.d}, p0/z, [x21, x0, lsl #3]   ## load next vector
  400e1c:       04f0e3e0        incd    x0                ## Increase k
  400e20:       65d82001        fadda   d1, p0, d1, z0.d  ## Perform partial sum
  400e24:       25f40c00        whilelo p0.d, w0, w20     ## Checking loop cond
  400e28:       54ffff81        b.ne    400e18            ## Branching on loop cond
~~~

However we will see that through reordering the loops the compiler will be able to use vector instructions more often.
Thus more operations can be done on the same data leading to better performance.

## c)

|       | Order   | MFLOPs   |
|-------|---------|----------|
| **1** | **ikj** | **5140** |
| 2     | kij     | 4587     |
| 3     | jik     | 2796     |
| **4** | **ijk** | **2628** |
| 5     | jki     | 649      |
| 6     | kji     | 194      |

When deciding which loop ordering would be beneficial for cache reusability we first took a look at the default ordering of *ijk*.
In that case *B* gets accessed sequentially in the innermost loop, while *A* in the second innermost loop. 
However we access a completely new row of *C* in each iteration of the innermost loop, which means that we potentially have to pull much more data. This is due to the matrix being saved as a one-dimensional array, leading to huge jumps when hooping between rows of a matrix.

To fix this problem we switch the ordering of *k* and *j*.
Now *C* gets accessed sequentially in the innermost loop and only requiring a row jump after a full iteration of the second innermost loop.
Additionally the access patterns of *A* and *B* just get flipped, which means that for this loop ordering all matrices get accessed in a cache-aware matter.

Applying our theoretical assumptions and comparing the results clearly shows, that this had an huge impact on performance:

> PUT DATA HERE

In addition the reordering allowed the compiler to be able to use the fully vectorized *FMAD* vector instruction.
This essentially means that instead of just computing the partial sum for one element of the vector as a scalar result we now perform this operation on each element, directly updating the entire vector with one instruction.

~~~
  400d60:       a5e0a021        ld1d    {z1.d}, p0/z, [x1]
  400d64:       a5e04040        ld1d    {z0.d}, p0/z, [x2, x0, lsl #3]
  400d68:       65e18440        fmad    z0.d, p1/m, z2.d, z1.d  // use of FMAD instruction
  400d6c:       e5e0e020        st1d    {z0.d}, p0, [x1]
  400d70:       04f0e3e0        incd    x0
  400d74:       0430e3e1        incb    x1
  400d78:       25f30c00        whilelo p0.d, w0, w19
  400d7c:       54ffff21        b.ne    400d60 <_Z2mmiid+0x1c0>  // b.any
~~~

**TODO:**
- Copy in measurements
- Update table from top

## d)

To circumvent the limitation that the TILE_SIZE must be a multiple of N we decided to pad the matrix to the next multiple.
This padding is implemented by just filling up the remaining rows and columns with zeros and has no effect on the result of the matrix multiplication.
However depending on the number of zero elements, this can have a significant impact on the overall execution time.
In general we have to compromise execution time for potentially better cache reusability by getting the possibility to make fine adjustments to the tile size.
During our testing we found out that having near zero overhead from padding is the best strategy.
In addition choosing a tile size that allows us to fit 3 tiles into the L1 cache seems to gain the best performance over different matrix sizes.

*TODO:*
 - Calculation for each tile size fitting into cache
 - Put calculations in
 - Results for each option with neighboring values

## e)