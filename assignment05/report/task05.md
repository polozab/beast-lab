# Task 5

## Traces measured

When taking a look at the measured traces by rocprof using `rocprof -- list - basic` we can extract sum useful counters.
For example we could check if faster FMA instructions are used by the GPU when computing the kernel:

~~~
SQ_INSTS_VALU_ADD_F64 : Number of VALU ADD/SUB instructions on float64. (per-simd, emulated)
SQ_INSTS_VALU_MUL_F64 : Number of VALU MUL instructions on float64. (per-simd, emulated)
SQ_INSTS_VALU_FMA_F64 : Number of VALU FMA/MAD instructions on float64. (per-simd, emulated)
SQ_INSTS_VALU_TRANS_F64 : Number of VALU transcendental instructions on float64. (per-simd, emulated)
~~~

Additionally for matrix size optimization we could take a look at the cache hits and misses:

~~~
# Shader/Sequencer Query Cache
SQC_DCACHE_HITS : Number of cache hits. (per-SQ, per-Bank, nondeterministic)
SQC_DCACHE_MISSES : Number of cache misses, includes uncached requests. (per-SQ, per-Bank, nondeterministic)

# Texture/Color Cache
TCC_HIT[0-31] : Number of cache hits.
TCC_MISS[0-31] : Number of cache misses. UC reads count as misses.
~~~

While TCC closely represents a L2 cache, the SQC ist only for responsible for instructions and scalar data. However we were not able to find according traces for the TCP cache wich would represent a Level 1 vector cache.

Looking at the derived metrics for rocperf we can spot many clarifying traces that could help us debug our still very poorly optimized kernel.
To help us diagnose the issue we could take a look at the stall traces measured when executing our program. We could for example verify that we are memory bound if the memory stalls reported by `MemUnitStalled` are high or the `BANDWIDTH_EA` trace is near the theoretical bandwidth limit. Additionally we could check if our tiling is optimal by checking the `L2CacheHit` and our `TCC_MISS` are low. And as a last example we could also calculate the floating point operations using `TOTAL_64_OPS`, `TOTAL_32_OPS`, `TOTAL_16_OPS` as counters fore executed FLOP operations.

There are many other interesting traces we could consider depending on what we observe and deduce as the current bottleneck of our kernel, but unfortunately due to our limited capacity we will only focus on the traces mentioned above for our observations.

## Program Observations

To be able to get precise measurement that only contain the matrix multiplication kernel we first ran a `hsa`-trace to verify that out `mm_kernel` was listed separately and can be therefore watched exclusively.

~~~
"Name","Calls","TotalDurationNs","AverageNs","Percentage"
"__omp_offloading_43_e270c8a__Z9mm_kernelPdS_S_i_l20.kd",1,249718841,249718841,98.82054097762018
"__omp_offloading_43_e270c8a__Z2mmiid_l61.kd",1,2980485,2980485,1.1794590223798223
~~~

We only looked at one matrix size 4000, were we previously reached out peak performance of around 500 GFLOPS/s.

Our first observation was that due to the profiling the performance of our kernel dropped significantly to around 476 - 472 GFLOPS/s depending on how many counters were measured.
Looking at the results we immediately saw that LDS is not used at all in our kernel. This means that the data is directly loaded from HBM and is not using the shared memory in any way. This directly fits other observations like the very high stall sums of the TCC and TCP and thus low busy times of the vector arithmetic units. Moreover it explains why our kernel performance saturates at around 350-500 GFLOPS/s and proves our previous assumption that we are still memory bound.
As a positiv at least we get a high cache reusability at the L2 level, which means that our tiling is at least in that sense effective. Also we have a nearly 100% SIMD utilization.

Our primary focus now for optimizing the kernel should be to make sure that LDS is used correctly and get the HBM bandwidth utilization down to reduce stalling times due to memory loads. 
