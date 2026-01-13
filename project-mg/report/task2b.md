# Task 2b - Parallelization

To optimize our current performance of `` we looked at the result being given by Nsight. For the following we wanted to node that we both used Linux Perf and Nsight simultaneously, however as Nsight delivered clearer visual results we chose to base this report the measurements and graphics provided by Nsight. In addition the source code using NVTX was much simpler to adopt as it didn't depend on any templating. In the following we show the unmodified kernel with just adding the NVTX calls to allow measuring the time of the individual stages:

>> Insert image

As we can clearly see most time is spend on the Jacobi iteration, which is why we chose to parallize it first.

## Jacobi Iteration

### Removing pointer usage and relying on index calculation

Although the code states that using pointer completely eliminates "expensive" index calculation we found that after reverting the code to an index based approach it didn't degrade performance in any significant way. It in some way even caused the opposite where we got minimally faster results from the index-based version. However these could also be caused by measurement inaccuracies. AAnother positive it made the kernel much more compact and easier to work with when parallizing.

As a first step we just add the following OMP-clause to distribute the iteration among threads assigning them to different cores:

```cpp
const unsigned int stride = _nx + 2;
#pragma omp parallel for schedule(static) 
for (unsigned int y = 1; y < _ny + 1; y++)
{
  for (unsigned int x = 1; x < _nx + 1; x++)
  {
    const unsigned int i = y * stride + x;
    writeField[i] = _RHS * rhs[i];
    writeField[i] += _X * (readField[i - 1] + readField[i + 1]);
    writeField[i] += _Y * (readField[i - stride] + readField[i + stride]);
  }
}
```

Adding `schedule(static)` to enforce almost overhead-free scheduling didn't make a huge difference and changing it to dynamic scheduling got us minimally slower performance. This is expected as we estimate the same computational latency for every different iteration which is why dynamic scheduling doesn't benefit us and only applies overhead from the scheduling.

Collapsing also didn't make a significant difference. We think this is due to the vectorization that could be applied if only one level was distributed over different threads. Now the compiler was free to vectorize the inner loop

## Prolongation

## Restriction

## Error calculation

> Still Bad cache behavior
