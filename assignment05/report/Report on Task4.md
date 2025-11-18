# Report on Task4
The implementation for Task 4 uses a basic approach for multi GPU matrix multiplication. All matrices are initialized on the CPU and then copied to each GPU. Each GPU computes a portion of the resulting matrix, and the final result is copied back to the CPU. 
> **Speculation:**
> 
> We assume that this approach is not the best as there is a lot of memory transfers between the host and the devices. A better approach would be to split the matrices on the devices directly without copying the full matrices to each GPU. But given our smaller groupsize we were not able to implement this approach.

## Implementation Configuration
- Variant: implicit column-major
- Number of Threads: 1024
- Number of Teams: 64
- compiler flags: clang++ -std=c++17 -Wall -Wextra -fopenmp --offload-arch=gfx90a -O3
## Measurements
![](/Group-101/assignment05/report/plots/multi_mean_range.png)

## Interpretation of Results
The measuremts show that multi GPU offloading can provide performance improvements for large matrix sizes. In the test all eight available GPUs were used. However the results are very unstable which is why the test was run 20 times. The results of this tests are in [this file](/Group-101/assignment05/report/plots/multi_summary.csv) where max, min and mean MFLOPs are given for each matrix size. The absolut top value was 913GFLOPs for N=4000.
### Speedup
![](/Group-101/assignment05/report/plots/speedup.png)

The speedup compared to the single GPU implementation is significant for large matrix sizes. However for smaller matrix sizes the overhead of managing multiple GPUs outweighs the benefits. Resulting in lower performance compared to the single GPU implementation. The best speed achieved was approximately 2.66x for the largest matrix size of N=4000. This speedup does not scale linearly with the number of GPUs used. But this was expected due to the overhead of data transfers and synchronization between the GPUs of our approach.