# Report on Matrix Profile Project

## Project Setup and configuration

To start off we had to decide on a build pipeline to compiler our code for the AMD GPU architecture. On AMD system the native compiler is the LLVM based clang compiler. However to make offloading and configuration simpler we used the AMD specific ROCm LLVM compiler infrastructure.[*](https://rocm.docs.amd.com/projects/llvm-project/en/latest/index.html)

```
AMD clang version 18.0.0git (https://github.com/RadeonOpenCompute/llvm-project roc-6.3.4 25012 e5bf7e55c91490b07c49d8960fa7983d864936c4)
Target: x86_64-unknown-linux-gnu
Thread model: posix
InstalledDir: /opt/rocm-6.3.4/lib/llvm/bin
Configuration file: /opt/rocm-6.3.4/lib/llvm/bin/clang++.cfg
```

We modified the Makefile to also link the ROCTracer API to ease the profiling process with AMD's ROCProfiler. In addition we used the following compiler flags to allow GPU offloading via OpenMP and apply standard optimization:

- `O3`: Enables default optimizations
- `target`: Define host (CPU) target architecture
- `fopenmp`: Enable OpenMP compiler support
- `offload-arch`: Shortcut defining the offloading (GPU) target architecture (AMD specific)

## Profiling

> **TODO**

## GFLOP calculation

To be able to compare different results of our code we had to use a unified measurement that could be evaluated easily. For that we measured the offloaded kernel runtime to calculate the archived FLOPS per second:

```cpp
// Precomputations and initializations

#pragma omp target data
{
  tb = omp_get_wtime();
  kernel(mp, mpi, ts, mu, df, dg, norm, mlen, sublen, num_blocks, mp_block, mpi_block, QT_block); // Offloaded to the GPU
  te = omp_get_wtime();
}

// Writing results

```

Note that we only included the computation kernel and excluded the data transfer from/to the offloading device. This was done by surrounding the entire benchmarking code in a omp data structure so that the data transfer begins/ends before/after the time measurements. In addition we also excluded any precomputation and initialization. 
We also did verify that this didn't impacted our measurements in any real-world relevant way, which was not the case. The data transfers and precomputations did take some time but within only a few seconds for matrices of sizes `2^20`. This allowed us to focus our calculation on main computation which was the most important part anyways.

> **TODO**: INSERT IMAGE WITH INDIVIDUAL PRECOMP CODE

We started of by calculating the FLOPS per pair `(i,j)` which was easily obtainable:

```cpp
for (int i=0;i<mlen-sublen;i++){
  for (int j=i+sublen;j<mlen ;j++){

    if (i!=0)
      QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];  // 4 FLOPS (only for i!=0)

    double cr = QT[j-i-sublen] * norm[i] * norm[j];  // 2 FLOPS

    if (cr > mp[i]){
      mp[i]=cr;
      mpi[i]=j;
    }

    if (cr > mp[j]){
      mp[j]=cr;
      mpi[j]=i;
    }
  }
}
```

We conclude `2 FLOPS` if `i=0` and `6 FLOPS` otherwise. Next we have to decide how many pairs `(i,j)` we visit. The outer loop we execute `mlen-sublen = N` times while the inner loop gets executed `mlen-sublen-i = N - i` times for every outer iteration `i`. Thus we get:

```r
sum(N-i) for i=0...(N-1) => N*(N+1)/2 iterations
```

Now we just have to differentiate the `i=0` cases, which is easily done as it only happens once for the first outer loop iteration.
In that case we execute the inner loop exactly `mlen-sublen-i = N - i = N` times as `i=0` according to our assumption. This leafs us with the following cases:

```r
# Always
N*(N+1)/2 iterations * 2 FLOPS

# i >= 1
N*(N+1)/2 - N = (N*(N+1)-2*N)/2 = (N*(N-1))/2 iterations * 4 FLOPS
```

Which we now just add up to obtain our final number of FLOPS:

```r
(N*(N-1))/2 * 4 + (N*(N-1))/2 * 4 = 2(N*(N-1)) + (N*(N-1)) = 3(N*(N-1)) = N*(3*(N-1)) FLOPS
```

To obtain the FLOP rate we just had to divide by the difference between both time measurements to obtain our final FLOPS/s value:

```cpp
const double t = te - tb;
const int N = mlen - sublen;
const int flops = (M - S) * (3*(M - S) - 1);
const double gflops = (double)flops / (t * 1e9);
```

The base CPU version got the follwing result which we want to improve on:

```
h039y10@milan2:~/beastlab/project-mp/src/mp> ./mp input/randomlist65536.txt 20 10
Runtime: 582.929375
GFLOP/S: 5.658

```

## GPU offloading

Our next task was to bring the kernel to the CPU which consisted of two parts: Data transfer between host and device and actual computation.

### Data transfer to offloading device

The data transfer between host and offloading device is handled via OpenMP `target data`-clauses.
We used the `map(to : ...)` directive to copy the precomputed data and the intermediate buffer `QT` to the offloading device. As these don't get changed by the computational kernel we wouldn't have to copy them back to host.
For the result buffers `mp`, `mpi` we also had to copy them back after finishing the computation, hence the usage of the `map(tofrom : ...)` directive.

We also tried to initialize the result and intermediate buffers only on the GPU to save copy time, however as this didn't matter for our GFLOPS anyways and every task done instead on the GPU would only worsen our results we kept it on the host side. In addition we examined the time it took for the initialized data to be transferred and it only took a few fractions of a second, which wouldn't make a real life difference in our case. Moreover we also did test changes in performance for our different approaches and in every one doing the initialization had no or a negative impact on the performance of that kernel.

Finally as the buffers are dynamically allocated memory regions we had to specifically tell OpenMP how long the buffer was to circumvent buffer overflows and incomplete copying of the buffers. This was done by stating the length on the variable name using `var[start:end]`.

The final data offloading clause looked like this:

```cpp
#pragma omp target data \
map(to: ts[:len], mu[:mlen], df[:mlen], dg[:mlen], norm[:mlen]) \
map(from: mp[:mlen], mpi[:mlen])
{
  tb = omp_get_wtime();
  kernel(mp, mpi, QT, df, dg, norm, mlen, sublen);
  te = omp_get_wtime();
}
```

### Computational kernel offload

The offload of the actual computational kernel is simple enough in OpenMP we just had to enclouse the entire code into a OMP `target` structure.
This would offload the kernel to the default offloading device, which was in our case the `GPU-8`.


In addition as usual we could instntiate teams of threads and threads itself using the `teams` and `parallel` directives. Loops iteration could than be distributed among teams and threads using `distribute` and `for` respectively. We will go into the specifics for any used offloading directive when we take a closer look at our different approaches.

As a baseline we executed the given kernel without any distributed or parallelized execution of the loop. However this took so long that we only took a measurement for a reduced matrix size:

```
h039y10@milan2:~/beastlab/project-mp/src/mp> ./mp input/randomlist65536.txt 10 10
Runtime: 0.312443
GFLOP/S: 0.010
```

As we can see on the GPU the performance is significantly worse due to the fact that it doesn't support any of the dynamic scheduling and out-of-order execution tricks done by the CPU. To be able to archive reasonable performance with a GPU we have to exploit it massive parallelism potential. However, before we could start parallelizing the kernel with our different approaches first we have to analyze the potential data races and with that we take a closer look at the loop dependencies next.

## Loop dependencies