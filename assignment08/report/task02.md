# Task 02

## Project and benchmark setup

For compilation of the kokkos kernels we used the same compiler setup from task 1. This task is also the reason why we used `gcc 13.2.0` instead of the newest `gcc` version, as there exists a incapability between `ipx` and the newest version of `gcc`. This only gets triggered from the compilation of the kernels library. 

We choose to build upon our best performing source file with RRL layout and updated the code by replacing the kernel functions for both CPU and GPU with the call to a predefined kokkos kernel. We selected the `gemm`-kernel from the Blas API, which gives us several options for multiplying two matrices:

```cpp
// C = beta * C + alpha * A * B
gemm(char transA, char transB, double alpha, View A, View B, double beta, View C)
```

For our benchmark we are only leveraging the simple `A * B` part without transforming any of the input matrices. Thus we set the scaling parameters to `1.0` and transA/B to `N`.

## Running the benchmark

To validate that the kokkos kernel returns correct results we verified each checksum against the checksum of our best-performing handwritten kernel. As they matched exactly on every case we assume that the calculations done by the kokkos kernel is correct. In addition we also performed several difference checks between the result matrices and found no deviations at all. Thus all calculated matrix norms were also the same (e.g. euclidean norm).

We decided to not use any repetition logic for the execution as these would only affect the results of very small matrix sizes anyway. Instead we chose to run the benchmark multiple times (in our case 5 runs per matrix size) to eliminate side effects and run variances by averaging the resulting measurements. For completeness we also recorded minimum and maximum values.

We also used the recommended flags `OMP_PROC_BIND=spread` and `OMP_PLACES=threads` from Kokkos for every execution of the binary.

## Results of the Kokkos kernel

