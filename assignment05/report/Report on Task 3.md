# Report on Task 3
## Implementation Configuration
- Variant: explicit column-major
- Size of N: 4000
- compiler flags: clang++ -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3

![task3_plot](/Group-101/assignment05/report/plots/heatmap.png)

## Hardware Configuration
- Every SIMD execution Unit (Wavefront) can execute up to 64 threads
- 104 Compute Units each serving one team

## Predictions
To fully utilize the GPU we need to have at least 104 teams to keep all compute units busy. Fewer teams would lead to underutilization of the GPU. Furthermore each team should have a number of threads that is a multiple of 64 to fully utilize the wavefronts.


## Observations
Our predictions correctly anticipated the performance trends observed in the heatmap. Having less than 104 teams, and not a multiple of 64 threads per team leads to significant performance drops. Having at least 256 teams already provides good performance, as most compute units can be kept busy and stalling wavefronts can be swapped out. One note worthy observation is that with 256 teams to achive the best performance 256 threads per team or are required.
However when using more teams than 256 the number of threads per team decreases to achieve the best performance. Our best results where achieved with 1024 teams and 64 threads per team achieving 306.367 GFLOPs. Although our next best results are very close to this value.