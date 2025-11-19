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

