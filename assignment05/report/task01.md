# Task 1 - Offloading with Optimal Data Transfer Policy

For offloading the data to the GPU we used a surrounding `omp target data`-clause which inhabits the data initialization and actual execution of the matrix multiplication kernel.
While the result Matrix `A` gets copied back after leaving the data region `B` and `C` only get allocated on the GPU.

For the data initialization we try to use the same loop setup as for the actual matrix to optimize the data accesses when loading the kernel. Although this is not as important as for the triad kernel due to the compute focus of matrix multiplication.
