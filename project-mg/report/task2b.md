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
#pragma omp parallel for
for (unsigned int x = 1; x < _nx + 1; x++)
{
  for (unsigned int y = 1; y < _ny + 1; y++)
  {
    const unsigned int i = y * stride + x;
    writeField[i] = _RHS * rhs[i];
    writeField[i] += _X * (readField[i - 1] + readField[i + 1]);
    writeField[i] += _Y * (readField[i - stride] + readField[i + stride]);
  }
}
```

After tweaking the scheduling strategy for the threads we found out that dynamic scheduling actually benefited our application. Using non-static scheduling improved our performance. In addition we tried an optimization that would only parallelize the loop if the covered iterations where large enough. This also increased our performance as we would save the overhead of initiating all the threads with the distributed data if it would only run over a very limited amount of iterations.

We also tried collapsing both loops with OMP. This however made the performance worse as it would take away the possibility to vectorize the inner loop. This could be seen in the output while compiling:

```
Jacobi::iterate(double const*, double*, double const*) const:
      8, include "Multigrid.h"
           8, include "Jacobi.h"
               24, Loop not vectorized/parallelized: contains call
               26, Loop not vectorized: data dependency
```

Where as if we didn't collapsed the loops it would say that it generated vectorized code. We would suspect distributing over the y-axis could break data locality for some of the iteration leading to worse performance than just looping over vectorized lines that lay in cache sequentially. (At least for the East and West values.)

Optimizing both the minimal number of iterations for parallization and the number of threads over which we distribute using `OMP_NUM_THREADS` gave us the following heatmap:

>> Insert heatmap

We there chose the number of minimal iterations to be `196608` and the number of threads to be `72` leaving us with the following kernel:

```cpp
void iterate(const FLOAT *const readField, FLOAT *const writeField, const FLOAT *const rhs) const
  {
    const unsigned int stride = _nx + 2;

    #pragma omp parallel for if (_nx * _ny >= 196608) schedule(dynamic)
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
  }
```

And we got the following measurements in Nsight:

>> insert image

Next we wanted to optimize either the prolongation or restriction part as both where now clearly the largest contributors to overall execution time.

## Prolongation

We chose to the interpolation kernel of the prolongation first. Again, we converted the pointer based approach to an index based one, for the same two reasons we encountered optimizing the Jacobi iteration. We also converted the scatter approach from the original kernel to an gather approach. This means that instead of iterating over each coarse grid cell and distributing (or scattering) this value to each fine grid cell, we go over every fine grid cell. With this method we fetch (or gather) in every iteration from the coarse grid values to set the according value for the fine grid cell. As a result we avoid any synchronization necessary when paralyzing the scatter approach to save overhead in that regard. This leaves us with the following interpolation kernel:

```cpp
for (unsigned int y = 1; y < _nyFineGrid + 1; ++y)
{
  for (unsigned int x = 1; x < _nxFineGrid + 1; ++x)
  {
    const unsigned int fineI = y * fineStride + x;
    const unsigned int coarseI = y / 2 * coarseStride + x / 2;
    
    const FLOAT wy = (y & 1u) ? 0.5f : 1.0f;
    const FLOAT wx = (x & 1u) ? 0.5f : 1.0f;
    
    fineGrid[fineI] +=
        wx * wy * coarseGrid[coarseI] +
        (1.0f - wx) * wy * coarseGrid[coarseI + 1] +
        wx * (1.0f - wy) * coarseGrid[coarseI + coarseStride] +
        (1.0f - wx) * (1.0f - wy) * coarseGrid[coarseI + coarseStride + 1];
  }
}
```

In addition we also applied dynamic scheduling and again gated small iteration counts from being parallelized at all. While this gave us improvements to our performance when trying to collapse both loops we worsened it like for the Jacobi iteration. Our final parallelized kernel looked like this:

```cpp
void interpolation(const FLOAT *const coarseGrid, FLOAT *const fineGrid) const
{
  const unsigned int fineStride = _nxFineGrid + 2;
  const unsigned int coarseStride = _nxCoarseGrid + 2;

  #pragma omp parallel for if (_nxFineGrid * _nyFineGrid >= 196608) schedule(dynamic)
  for (unsigned int y = 1; y < _nyFineGrid + 1; ++y)
  {
    for (unsigned int x = 1; x < _nxFineGrid + 1; ++x)
    {
      const unsigned int fineI = y * fineStride + x;
      const unsigned int coarseI = y / 2 * coarseStride + x / 2;
      
      const FLOAT wy = (y & 1u) ? 0.5f : 1.0f;
      const FLOAT wx = (x & 1u) ? 0.5f : 1.0f;
      
      fineGrid[fineI] +=
          wx * wy * coarseGrid[coarseI] +
          (1.0f - wx) * wy * coarseGrid[coarseI + 1] +
          wx * (1.0f - wy) * coarseGrid[coarseI + coarseStride] +
          (1.0f - wx) * (1.0f - wy) * coarseGrid[coarseI + coarseStride + 1];
    }
  }
}
```

When measuring performance we saw the following very substantial improvement using Nsight:

>> Add image

The next logical step was to take a look at the restriction kernel.

## Restriction



## Error calculation

> Still Bad cache behavior
