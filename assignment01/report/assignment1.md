# Assignment 1 - General Questions

## a) Floating Point Operations per Iteration

Each iteration of consists of 2 floating point operations. If we unpack the statement we can clearly spot both operation:

```
A[i] = (B[i] + (C[i] * D[i]))
```
The first one being the multiplication of *C + D* and the second being th addition with *B*. As storing and loading of floating point numbers doesn't count as a floating point operation the assignment to *A* is not considered.

Thus to calculate the floating point operations per second FLOPs we just measure:

```
FLOPs = (2N) / seconds_needed
```

## b) Conversion to bandwidth utilization
### i) TODO: CACHE SIZE
Each value of the vectors *B, C, D* needs to be loaded from memory, while storing the result in the value of *A* also writes to the memory. As double preciscion floating point numbers take up 8 Bytes and we need 4 memory accesses, 32 Bytes get transferred each iteration.

```
8 bytes * 4 transfers = 32 bytes
```

As we have 2 floating point operations per iteration we get:

```
32 bytes / 2 FLOP = 16 bytes / FLOP
```

Thus we can just multiply the FLOPs with 16 to get the memory utilization in bytes per second, as each FLOP equals 16 byte.

### ii)

In write allocate the CPU does an extra memory transfer if the desired address can not be found in the cache yet. It then loads in the entire cache line first, overwriting it with the value in cache. Finally if the cache line get evicted the expected memory transfer happens.

This means when using write allocate we get an additional memory transfer from th loading of the cache line into the cache. Thus a better estimation for the factor would be 20.

```
8 bytes * 5 transfers = 40 bytes
=> 40 bytes / 2 FLOP = 20 bytes / FLOP
```

## c) Code measurement

### i)

The repetition count decides how often the triad operation is run which itself loops over every entry of the vectors *A, B, C, D*. As one single full run of the triad operation can be very fast depending on the number of elements in one vector, repeating the same operation multiple times can improve measurement accuracy. 

This is due to the fact that small anomalies through for example other system processes, timer inaccuracies, machine accuracy or overhead from creating the thread team get averaged out over the runtime off all repetitions.

### ii)

Additionally to setting the repetition count to a number that the timing test lasts at least the specified minimal time, it also serves as a warm-up. This eradicates inaccuracies from cache cold misses and ensures threads are already allocated to one resource.

### iii)

The verification of the results makes sure that the vector *A* is at least once used, so the compiler doesn't optimizes the whole timing test out. It also serves as a safeguard that the parallelization is done correctly and doesn't output unexpected or wrong values.

## d) Optimization

### i)

The use of *sysconf(_SC_LEVEL1_DCACHE_LINESIZE)* ensures that we allocate the bases of the vectors *A, B, C, D* according to the cache line boundaries of the level 1 cache. As the entires itself match to the length of the cache line we prevent cross line accesses. This can improve performance as it may prevent an additional memory access due to a double value spreading over the end of a cache line.

```
64 bytes line length % 8 byte double size = 0
```

### ii)

*restrict* tells the compiler that the memory the pointer is pointing at will always only be accessed through this very pointer. This means the compiler will know that accesses to entries of *A, B, C, D* will always be independent and thus can be executed in a arbitrary order, allowing parallelization of statements like the triad operation.

## e) Parallelization

### i)

Due to the uniform nature of the triad operation it is useful to access memory in local chunks to improve cache locality. Through the use of the static scheduling of OpenMP threads get allocated in contiguous, equal-sized chunks which is optimal for cache locality and additionally has very little management overhead. 

As there are no dependencies between iterations or even repetitions we can use the *nowait* flag to ignore the implicit join barrier. This lets threads instantly continue with the next repetition saving unnecessary idle time.

Creating the parallel region outside the repetition loop reduces overhead from creating and destroying thread teams. This ensures the thread team is only created once per execution of the timing test instead of every time a new repetition starts. As the repetition is only needed to stabilize the timing results extra overhead would only distort the results.

### ii)

The initialization should be parallelized to make optimal use of the NUMA Domains. In case the parallelization would be done sequentially every vector entry would live in the NUMA Domain where the sequential thread got executed. This is due to the first touch policy of memory allocation.

Instead, if we run the initialization in parallel, too. We allocate the vector entries in the same NUMA domain where we later perform the triad operation. This can lead to better performance as accesses to memory are physically shorter, leading to less idle time.

This is due to the use of the same scheduling strategy for both initialization and actual operation. Even tho the for loops live in completely separate thread teams the deterministic scheduling of OpenMP ensures that the threads get allocated identically both times.