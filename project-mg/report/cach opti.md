Time limited we were only able to implment a part of a sequeitnailly optimized version so 
NX= 2048 
118,211,274,584      cache-references                                                      
     5,672,755,112      cache-misses                     #    4.799 % of all cache refs       
   120,308,175,408      cycles                                                                
   255,483,289,553      instructions                     #    2.12  insn per cycle            

      36.666344678 seconds time elapsed

      36.336724000 seconds user
       0.324006000 seconds sys
NX=512
118,366,340,102      cache-references                                                      
     6,029,199,141      cache-misses                     #    5.094 % of all cache refs       
   113,016,084,853      cycles                                                                
   257,476,482,737      instructions                     #    2.28  insn per cycle            

      34.539562639 seconds time elapsed

      34.166287000 seconds user
       0.367981000 seconds sys
so less cach != faster 

Unopti
179,007,764,547      cache-references                                                      
     6,253,210,979      cache-misses                     #    3.493 % of all cache refs       
   134,530,746,552      cycles                                                                
   376,580,293,866      instructions                     #    2.80  insn per cycle            

      41.004250532 seconds time elapsed

      40.697469000 seconds user
       0.300010000 seconds sys

       
# Cache Optimization
In order to decrease the execution time more, the amount of cach misses needs to be adressed.
Our first variant is based on the sequential default code.

Baseline:
```
179,007,764,547      cache-references                                                      
     6,253,210,979      cache-misses                     #    3.493 % of all cache refs       
   134,530,746,552      cycles                                                                
   376,580,293,866      instructions                     #    2.80  insn per cycle            

      41.004250532 seconds time elapsed

      40.697469000 seconds user
       0.300010000 seconds sys
```

By partitionint our rows in tiles `TILE_X` and then computing (if possible) two y values at once we can reduce the amount of cache misses due to the temporal locality of the values.
```cpp
for (unsigned int xBlock = 1; xBlock <= _nx; xBlock += TILE_X)
    {
      const unsigned int xEnd = std::min(_nx, xBlock + TILE_X - 1);

      {
        const FLOAT *c   = readField   + y * stride + xBlock;
        const FLOAT *s   = c - stride;                 // y-1
        const FLOAT *n   = c + stride;                 // y+1
        const FLOAT *r   = rhs        + y * stride + xBlock;
        FLOAT *out       = writeField + y * stride + xBlock;

        #pragma omp simd
        for (unsigned int x = xBlock; x <= xEnd; ++x)
        {
          *out = RHS * (*r)
               + X * (c[-1] + c[1])
               + Y * (s[0]  + n[0]);
          ++c; ++s; ++n; ++r; ++out;
        }
      }

      if (hasRow2)
      {
        const unsigned int y2 = y + 1u;

        const FLOAT *c   = readField   + y2 * stride + xBlock;
        const FLOAT *s   = c - stride;                // y
        const FLOAT *n   = c + stride;                // y+2
        const FLOAT *r   = rhs        + y2 * stride + xBlock;
        FLOAT *out       = writeField + y2 * stride + xBlock;

        #pragma omp simd
        for (unsigned int x = xBlock; x <= xEnd; ++x)
        {
          *out = RHS * (*r)
               + X * (c[-1] + c[1])
               + Y * (s[0]  + n[0]);
          ++c; ++s; ++n; ++r; ++out;
        }
      }
    }
```
When runing this code we found that the best TILE_X value is 512.
```
perf stat -e cache-references,cache-misses,cycles,instructions ./multigrid 4 6 14 8
Iteration 0, Max-error: 0.0162047
Iteration 1, Max-error: 0.000339175
Iteration 2, Max-error: 7.56442e-06
Iteration 3, Max-error: 1.76332e-07
Iteration 4, Max-error: 4.23933e-09
Iteration 5, Max-error: 1.07272e-10
Iteration 6, Max-error: 5.91649e-12
Iteration 7, Max-error: 4.38627e-12

 Performance counter stats for './multigrid 4 6 14 8':

   118,221,964,744      cache-references                                                      
     5,323,272,571      cache-misses                     #    4.503 % of all cache refs       
   119,214,814,285      cycles                                                                
   257,183,438,481      instructions                     #    2.16  insn per cycle            

      35.677928021 seconds time elapsed

      35.357383000 seconds user
       0.315976000 seconds sys
```
This results in a reduction of total cache misses by about 14,87% (the percentual amount of cach misses increased from 3.4 to 4.5% however) and a speedup of 14.9% (this indicates that our optimization is indeed effective).

When using the cach optimized `Jacobi.h`in our optimal OMP optimized version and adding simple OMP parallelization to the above code we achieve even better results:

```
Iteration 0, Max-error: 0.0162047
Iteration 1, Max-error: 0.000339175
Iteration 2, Max-error: 7.56442e-06
Iteration 3, Max-error: 1.76333e-07
Iteration 4, Max-error: 4.23909e-09
Iteration 5, Max-error: 1.10026e-10
Iteration 6, Max-error: 5.46702e-12
Iteration 7, Max-error: 4.7411e-12

 Performance counter stats for './multigrid 4 6 14 8':

   232,555,059,681      cache-references                                                      
     6,039,376,359      cache-misses                     #    2.597 % of all cache refs       
   764,534,296,235      cycles                                                                
   789,903,402,475      instructions                     #    1.03  insn per cycle            

       3.474389658 seconds time elapsed

     214.724026000 seconds user
      14.987062000 seconds sys
```
So compared to our best OMP optimized version we were slightly slower (this is because due to group size limitation we could not fully optimize our cach aproach as much as we did our OMP version)

We noticed that an decrease in cach misses dosnt necessarily lead to a faster execution time as shown in the following examples: 
Tile size 128:
``` 
   235,294,290,336      cache-references                                                      
     6,005,908,637      cache-misses                     #    2.553 % of all cache refs       
   789,042,932,024      cycles                                                                
   801,316,112,342      instructions                     #    1.02  insn per cycle            

       3.579061896 seconds time elapsed
```
Tile size 8192:
```
208,315,378,717      cache-references                                                      
     6,437,643,045      cache-misses                     #    3.090 % of all cache refs       
   742,223,048,634      cycles                                                                
   697,426,200,982      instructions                     #    0.94  insn per cycle            

       3.385407431 seconds time elapsed
```
Tile size 99999:
```
Performance counter stats for './multigrid 4 6 14 8':

   208,300,155,729      cache-references                                                      
     7,720,919,725      cache-misses                     #    3.707 % of all cache refs       
   736,100,412,016      cycles                                                                
   710,040,887,482      instructions                     #    0.96  insn per cycle            

       3.360213680 seconds time elapsed

     208.907381000 seconds user
      12.330964000 seconds sys
```

for x = one row:
```
Performance counter stats for './multigrid 4 6 14 8':

   213,865,765,156      cache-references                                                      
     7,936,013,233      cache-misses                     #    3.711 % of all cache refs       
   740,495,728,032      cycles                                                                
   731,932,495,397      instructions                     #    0.99  insn per cycle            

       3.374978329 seconds time elapsed

     209.212631000 seconds user
      13.383126000 seconds sys
```
What we noticed that the row spliting dosnt add to much performance increase so for x = _nx (one entire row) single thread
```
 118,652,339,741      cache-references                                                      
     7,744,634,757      cache-misses                     #    6.527 % of all cache refs       
   111,266,150,630      cycles                                                                
   351,123,491,827      instructions                     #    3.16  insn per cycle            

      33.298175094 seconds time elapsed
```

X=512
```
Performance counter stats for './multigrid 4 6 14 8':

   119,227,422,619      cache-references                                                      
     5,555,778,567      cache-misses                     #    4.660 % of all cache refs       
   122,632,485,860      cycles                                                                
   338,977,355,638      instructions                     #    2.76  insn per cycle            

      36.761167999 seconds time elapsed

      36.338196000 seconds user
       0.419979000 seconds sys
```

SO tldr less cach misses and references != faster