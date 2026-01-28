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

For profiling our applications we used the ROCProfiler[*](https://rocm.docs.amd.com/projects/rocprofiler/en/latest/index.html) in combination with the ROCTracer API[*](https://rocm.docs.amd.com/projects/roctracer/en/latest/doxygen/html/index.html).

This allowed us to define custom execution ranges similar to our last project with Nsight. In Addition we also collected counters and events. For that the  [`rocprof`](https://rocm.docs.amd.com/projects/rocprofiler/en/latest/how-to/using-rocprof.html#) command line tool is used.

### RocTX Annotations

To be able to use the `--roctx-trace` option effectively and allow the API to show us concrete time ranges we first had to annotate our source code by using the `roctxRangePush` and `roctxRangePop` methods that we include using the provided shared library. We used this to annotate the different phases of the application: initialization, calculate precomputed buffers, mapping data to offloaded device, execution of the computational kernel, and transferring the data back.
This allowed us to precisely check the individual execution times of certain ranges and decide what we should focus on.

In the following we see an example were we captured the individual calculations of the precomputed fields using RocTX annotation:

```cpp
// prepare statistics, initialization (some needed for the first row of matrix)
roctxRangePush("MU");
precom_mu(ts,mu,mlen,sublen);
roctxRangePop();
roctxRangePush("NORM");
precom_norm(ts,norm,mu,mlen,sublen);
roctxRangePop();
roctxRangePush("DF");
precom_df(ts,df,mlen,sublen);
roctxRangePop();
roctxRangePush("DG");
precom_dg(ts,mu,dg,mlen,sublen);
roctxRangePop();
```

To run the tracer we issued the following command:

```sh
rocprof --roctx-trace ./mp input/randomlist65536.txt 16 10
```

And the following results showing that as expected calculating the norm takes the longest time among those methods:

> **TODO**: Image of precomputations

### Collecting system counters

The other part useful for optimizing the matrix profile kernel is the ability for RocProf to record system counter like cache misses and used memory bandwidth. For this we had to supply the `rocprof` CLI tool with an input file defining several counters we want to measure and limitations to the context we want to capture. With the later one it was possible to specifically only measure the offloaded part we were interested in. 

To obtain the list of collectable counter the `rocprof` CLI can be used with the `--list-basic` and `--list-derived` option. After checking the available counter we decided to capture the following ones:

- `GPUBusy`: Shows if the GPU is actually utilized
- `VALUBusy`: Check whether the functional units actually process something or are bottlenecked by e.g. memory
- `OccupancyPercent`: Similar check for occupancy to determine bottlenecks
- `BANDWIDTH_EA`: Check whether we are bandwidth limited
- `MemUnitStalled`: Shows if the memory units are fully utilized or stalled due to some reason
- `TOTAL_64_OPS`: As a comparison to check whether the GFLOP approximation done in the next part is representative
- `L2CacheHit`: Analyze how good the cache reuse for the executed code is 

The finished input file only capturing the main computational kernel (named `kernel` in the source code) looked like this:

```
pmc: GPUBusy, VALUBusy, OccupancyPercent, BANDWIDTH_EA, MemUnitStalled, TOTAL_64_OPS, L2CacheHit
kernel:kernel
```

The profiler could be issued with following command including the previous input file and mentioned options:

```sh
rocprof --stats -i ./input.txt ./mp input/randomlist65536.txt 16 10
```

The result could be views on the following website[*](https://ui.perfetto.dev/) by uploading the created `results.json` file.

In the future it would also be helpful to capture the `HIP` trace to be able to see the individual kernels on the GPU and also allow to use annotations within offloaded methods. However this was not possible with OpenMP offload instead and we would have to use actual `HIP` kernels to be able to archive this.

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

In the main computational kernel there are two dependencies present. The first one is connected to the accumalation of the intermediate `QT` buffer. It holds the diagonal state of the matrix which is currently being processed. Due to the iteration variables it gets visited by multiple unique `(i,j)` pairs. In the approaches we propose different methods of circumventing this dependency.

```cpp
for (int i=0;i<mlen-sublen;i++){
  for (int j=i+sublen;j<mlen ;j++){

    if (i!=0)
      QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];

    double cr = QT[j-i-sublen] * norm[i] * norm[j];
    ...
  }
}
```

For example in the previous code lets say `sublen=2`. If we visualize the indices of `QT[j-i-sublen]` for each pair `(i,j)` according to the loop variables we see that  almost all offsets within `QT` are accessed multiple times. However we also see that there is a certain diagonal pattern that could be exploited by letting a single thread own such an diagonal. In that case no race conditions would occur which we will also employ in one of our approaches:

```
        j
        →
        0 1 2 3 4 5 6 7
i ↓  0  . . . . . . . .
     1  . . . 0 1 2 3 4
     2  . . . . 0 1 2 3
     3  . . . . . 0 1 2
     4  . . . . . . 0 1
     5  . . . . . . . 0
     6  . . . . . . . .
     7  . . . . . . . .
```

The second dependency is caused by the updates to the result fields `mp` and `mpi`. This is due to the non-atomicity of the conditional updated which causes a race conditions if multiple threads passed the max-check based on an outdated values and than write their own value into the field. This could most definitely not be the actual maximum value considering all present threads that checked the max-check. We will also explain solutions for this problem in the individual approaches we tried.

```cpp
if (cr > mp[i]){
  mp[i]=cr;
  mpi[i]=j;
}

if (cr > mp[j]){
  mp[j]=cr;
  mpi[j]=i;
}

```

As an example let `mp[i] = 67.3`. Now a thread could pass the max-test with the value `69.3`. Now simultaneously a separate thread could pass the max-test with `67.8`, and depending on which thread now writes last the later thread could actually persist in the result field although it was not the maximum value.

## Different approaches to parallelization 

In the last two weeks we tried out two different main approaches. Each can be found in a separate files:

**[Approach 1](approach1.md)** (includes `main_1a.cpp` and `main_1b.cpp`)

**[Approach 2](approach2.md)** (includes `main_2.cpp`)
