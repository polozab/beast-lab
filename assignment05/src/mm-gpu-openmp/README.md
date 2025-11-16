# Readme
## Building
The benchmarks can be build simply using `make`. Optionally the environment variables setting the number of teams and threads can be set with `NUM_TEAMS` and `NUM_THREADS`respectively. These will be used at compile time to specify the `num_teams` and `num_threads` directives for OpenMP. They both default to `64`.

For the implicit benchmarks which utilize the `loop` directive these environment variables will not be used to give the compiler and runtime maximal freedom. Instead if explicit values are necessary for these only the `mm-gpu-offload_implicit_col_limit.cpp` file will use `NUM_TEAMS` and `NUM_THREADS` as values for the `num_teams` and `thread_limit` directives.

### Prod Benchmark
This is the default target of the Makefile. Only reads teams and threads based on the environment variables `NUM_TEAMS` and `NUM_THREADS`. No further runtime query will be executed to validate the use of the specified values. Thus in the result logs the `Actual_Teams` and `Actual_Threads` will default to -1.

### Debug Benchmark
This target enables an additional runtime test to verify the used teams and threads. For this test `omp_get_num_teams` and `omp_get_num_threads` are logged as `Actual_Teams` and `Actual_Threads` in the result. Note that due to limitation this verify code runs outside of the actual kernel region in a separate one, and logged values should be taken with care.
