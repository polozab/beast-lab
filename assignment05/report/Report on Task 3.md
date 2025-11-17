# Report on Task 3
## Implementation Configuration
- Variant: explicit column-major
- Size of N: 4000
- compiler flags: clang++ -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3

![task3_plot](/Group-101/assignment05/report/plots/heatmap.png)

## Hardware Configuration
- each Wavefront executes 64 threads
- 104 Compute Units per die


## Observations
Less than 104 teams result in underutilization of the GPU therefore the performance is poor.

Less than 64 threads per team also result in underutilization due to lane underutilization. 

Having more than 104 teams gives the scheduler enough work to swap in when some waves stall on memory.

The best performance is achieved with 1024 teams and 64 threads per team which results in a performance of approximately 307 GFLOPs. This is because with 1024 teams there is enough work to keep the GPU busy and with 64 threads per team all lanes of a wavefront are utilized.

256 teams with more than 256 threads performs also very well 
> todo: finish this sentence