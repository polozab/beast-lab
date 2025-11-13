# Task 2 - Evaluating Data Transfer

## Setup

### CPU initialization

For the CPU initialization we first allocate the four vectors. These get used for the subsequent value initialization. Both get fully executed on the CPU side.

~~~c++
double *a = new double[N];
double *b = new double[N];
double *c = new double[N];
double *d = new double[N];

for (size_t i = 0; i < N; i++)
{
    a[i] = b[i] = c[i] = d[i] = i;
}
~~~

After that we initialize the data region using mappings to ensure the vectors get transferred to and from the GPU on entering and exiting the region respectively. For that we can use the `omp target data` directives and the `map` directive.
We use a *tofrom* mapping for the vector that we are writing to and for all the other one only a *to* mapping, as we don't bother about the unchanged values in the vectors *b*,*c* and *d*.

~~~c++
#pragma omp target data map(tofrom:a[0:N]) map(to:b[0:N],c[0:N],d[0:N])
~~~

One important thing to notice is, that we have to explicitly tell the compile the range of the vectors by providing the indices. 
Without this specification we would run into segmentation faults, as the memory doesn't get allocated correctly.

We put the data mappings outside the repetition loop to circumvent unnecessary data transfers between individual repetitions. 
As the data doesn't need to be written back after every repetition we just copy it once to the GPU at the beginning and write back *a* after finishing the whole measurement. 
This saves significant amounts of bandwidth and thus also improves performance as transfer latencies between repetitions get eliminated.

### GPU initialization

When we want to initialize the memory used for computation on the GPU we again have to initialize on the CPU side first. 
However this time only for reference as we don't copy any values written to them to the GPU. 
Thus we also skip the value initialization loop in this case.

After initializing the memory on the CPU side we again perform the data mapping, but this time using different mapping types. 
The vectors *b*, *c* and *d* only ger allocated on the GPU side, which means that no transfer at all happens. 
As we are not interested in the values on the CPU side this is completely fine. For the result vector *a* we also don't copy over the data to the GPU, as we will initialize it alongside the other vectors on the GPU. 
We only care about the resulting value on the CPU side, thus we are using the *from* mapping to copy it over to the CPU when the measurements are finished.

~~~c++
#pragma omp target data map(from:a[0:N]) map(alloc:b[0:N],c[0:N],d[0:N])
~~~

To initialize the values we now execute the same loop on the GPU side by using the `omp target` directive. We also parallelize this operation with `teams distribute` and `parallel for` in the same way as we later execute the actual triad kernel.

~~~c++
#pragma omp target teams distribute parallel for
for (size_t i = 0; i < N; i++)
{
    a[i] = b[i] = c[i] = d[i] = i;
}
~~~

After this point we just continue as normal and start the performance measurements.

## Evaluation

![alt text](task1-plots/memory-init.png)

### Observation

We can see an enormous performance improvement when initializing the vectors on the GPU instead of the CPU.
The performance for the CPU peaks at around 16481.01 MFLOPS at a size of 524288 elements per vector. Until this point we can see the same scaling curve to the GPU initialization however the increase is much flatter.
After the peak it drops significantly and settles at around 8000 MFLOPS decreasing slowly.

On the GPU side we can see that the scaling continues and doesn't get stopped like on the CPU. It saturates at around 1073741824 elements with a peak performance of 237008.73 MFLOPS.
Although the scaling may continue as we weren't able to test with larger vector sizes due to reaching memory limitations.
Noticeable we see a small plateau at the same vector size of 524288 for the GPU initialization were the CPU initialization did drop too.

### Explanation

Similar to the improvements on the CPU side when allocating memory near threads that later use the data the GPU can also benefit massively form location-aware memory allocation.
When we copy the data over from the CPU to the GPU it is not guaranteed that we transfer it to memory that is close to the cores operating on this data. This could significantly decrease performance due to longer waiting times for the GPU memory.

If we then instead initialize the memory directly on the GPU with the same teams and thread using deterministic scheduling, we also enure that the relevant vector entries get stored in local memory near to the executing cores.
As OpenMP uses deterministic scheduling we can profit substantially from fast local memory accesses, which especially improves performance for memory heavy kernels like triad.

In addition the data can be loaded into the shared cache of one streaming multiprocessor, which benefits performance even more when the teams using this data get assigned to the same streaming multiprocessor both times during initialization and kernel execution.
