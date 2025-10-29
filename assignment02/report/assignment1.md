# Task 1 - Theoretical foundation

## a)

Both of the outer loops can be parallelized, whilst the innermost must be executed sequentially due to data dependencies on *A*.
There, the field *A[i][j]* gets read and written to *k* times which would lead to race conditions if parallelized.
However the outer loops just assign an new data point *A[m][n]* which is unique for ever iteration of both loops.

## b)

The outer loops can be parallelized using the `#pragma omp parallel for` directive.
Here it would even be possible to use `collapse(2)` to parallelize both loops at once since they are perfectly nested.
In sequential execution changing the schedule of loops may help with cache locality.
However due to the fact that both loops are parallelized this does not matter here.
The inner loop still cannot be parallelized due to previously mentioned data dependencies.
