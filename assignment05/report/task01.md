# Task 1 - Offloading with Optimal Data Transfer Policy

For offloading the data to the GPU we used a surrounding `omp target data`-clause which inhabits the data initialization and actual execution of the matrix multiplication kernel.
While the result Matrix `A` gets copied back after leaving the data region `B` and `C` only get allocated on the GPU.

~~~c++
#pragma omp target data map(from : a[0 : N_sqr]) map(alloc : b[0 : N_sqr], c[0 : N_sqr])
~~~

For the data initialization we try to use the same loop setup as for the actual matrix to optimize the data accesses when loading the kernel. Although this is not as important as for the triad kernel due to the compute focus of matrix multiplication.

~~~c++
#pragma omp target
#pragma omp teams num_teams(NUM_TEAMS)
#pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(static, 1)
for (int i = 0; i < N; ++i)
{
    for (int j = 0; j < N; ++j)
    {
        a[IDX(i, j, N)] = 0.0;
        b[IDX(i, j, N)]= (i * j) / (N * 50);
        c[IDX(i, j, N)] = (i * j) / (N * 50);
    }
}
~~~

Using `static` scheduling for the initialization and `dynamic` scheduling for the kernel improves performance slightly. We suspect that the `dynamic` scheduling allows the GPU to load balance the chunks of the matrix a little better. Especially because is isn't yet stored in the much more efficient column major order.

~~~c++
#pragma omp target
#pragma omp teams num_teams(NUM_TEAMS)
#pragma omp distribute parallel for num_threads(NUM_THREADS) collapse(2) schedule(dynamic, 1)
for (int i = 0; i < N; ++i)
{
    for (int j = 0; j < N; ++j)
    {
        for (int k = 0; k < N; ++k)
        {
            a[IDX(i, j, N)] += b[IDX(i, k, N)] * c[IDX(j, k, N)];
        }
    }
}
~~~

We tried several other combination, however this advantage only was visible using this specific combination. Although we would also suspect similar results with only specifying the `dynamic` kernel scheduling. This however was not the case and unfortunately we didn't have time to look into this further. Luckily these advantages of different scheduling types only appeared for this very unoptimized version.
