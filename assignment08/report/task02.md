# Task 02

## Project and benchmark setup

For compilation of the kokkos kernels we used the same compiler setup from task 1. This task is also the reason why we used `gcc 13.2.0` instead of the newest `gcc` version, as there exists a incapability between `ipx` and the newest version of `gcc`. This only gets triggered from the compilation of the kernels library. 

We choose to build upon our best performing source file with RRL layout and updated the code by replacing the kernel functions with the call to a predefined kokkos kernel. We selected the `gemm`-kernel from the Blas API, which gives us several options for multiplying two matrices:

```cpp
// C = beta * C + alpha * A * B
gemm(char transA, char transB, double alpha, View A, View B, double beta, View C)
```

For our benchmark we are only leveraging the simple `A * B` part without transforming any of the input matrices. Thus we set the scaling parameters to `1.0` and transA/B to `N`.

## Running the benchmark

To validate that the kokkos kernel returns correct results we verified each checksum against the checksum of our best-performing handwritten kernel. As they matched exactly on every case we assume that the calculations done by the kokkos kernel is correct.