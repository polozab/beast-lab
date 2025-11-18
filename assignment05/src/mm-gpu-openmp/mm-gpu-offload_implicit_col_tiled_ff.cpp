// Simple tiled version (column-major C), mapping once and reusing data.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <omp.h>

#ifndef TILE_SIZE
#define TILE_SIZE 32
#endif

#define IDX(row, col, width) ((width) * (row) + (col))
#define IDX_COL(row, col, height) ((row) + (col) * (height))

static void mm_kernel_tiled(double *a, double *b, double *c, int N, int padded_N)
{
    #pragma omp target teams distribute collapse(2)
    for (int ii = 0; ii < padded_N; ii += TILE_SIZE)
    {
        for (int jj = 0; jj < padded_N; jj += TILE_SIZE)
        {
            for (int kk = 0; kk < padded_N; kk += TILE_SIZE)
            {
                #pragma omp parallel for collapse(2)
                for (int i = 0; i < TILE_SIZE; ++i)
                {
                    for (int j = 0; j < TILE_SIZE; ++j)
                    {
                        double sum = 0.0;
                        for (int k = 0; k < TILE_SIZE; ++k)
                        {
                            sum += b[IDX(ii + i, kk + k, padded_N)] *
                                   c[IDX_COL(jj + j, kk + k, padded_N)];
                        }
                        a[IDX(ii + i, jj + j, padded_N)] += sum;
                    }
                }
            }
        }
    }
}

double mm(int N, int REP, double expected_result)
{
    int tiles = (N + TILE_SIZE - 1) / TILE_SIZE;
    int padded_N = tiles * TILE_SIZE;
    const long padded_sz = (long)padded_N * padded_N;

    double *a = (double *)malloc(padded_sz * sizeof(double));
    double *b = (double *)malloc(padded_sz * sizeof(double));
    double *c = (double *)malloc(padded_sz * sizeof(double));
    if (!a || !b || !c) { fprintf(stderr, "alloc failed\n"); exit(EXIT_FAILURE); }

    // Host init with padding
    for (int i = 0; i < padded_N; ++i)
    {
        for (int j = 0; j < padded_N; ++j)
        {
            bool in = (i < N) && (j < N);
            a[IDX(i, j, padded_N)] = 0.0;
            b[IDX(i, j, padded_N)] = in ? (i * j) / (N * 50) : 0.0;
            c[IDX_COL(i, j, padded_N)] = in ? (i * j) / (N * 50) : 0.0;
        }
    }

    #pragma omp target enter data map(to: b[0:padded_sz], c[0:padded_sz]) map(alloc: a[0:padded_sz])

    // Zero A on device
    #pragma omp target teams loop collapse(2) map(present, tofrom: a[0:padded_sz])
    for (int i = 0; i < padded_N; ++i)
        for (int j = 0; j < padded_N; ++j)
            a[IDX(i, j, padded_N)] = 0.0;

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < REP; ++r)
    {
        mm_kernel_tiled(a, b, c, N, padded_N);
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    #pragma omp target exit data map(from: a[0:padded_sz]) map(delete: b[0:padded_sz], c[0:padded_sz])

    using dsec = std::chrono::duration<double>;
    double dur = std::chrono::duration_cast<dsec>(t1 - t0).count();

    // Sum only valid N×N for verification
    double sum = 0.0;
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            sum += a[IDX(i, j, padded_N)];

    if (std::abs(sum - expected_result) > 0.001)
    {
        printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n",
               N, expected_result, sum);
        exit(EXIT_FAILURE);
    }

    free(a); free(b); free(c);

    double gflops = (2.0 * (double)N * (double)N * (double)N * (double)REP * 1.0e-9) / dur;
    printf("%d, %d, GFLOPs=%f\n", N, REP, gflops);
    return gflops;
}

int main()
{
    printf("N, REP, GFLOPs\n");
    mm(100, 2000, 108290000.000);
    mm(200, 250, 1170799500.000);
    mm(300, 75, 3560702175.000);
    mm(400, 32, 7353887776.000);
    mm(500, 16, 12171228800.000);
    mm(750, 10, 64265028760.000);
    mm(1000, 6, 171249832524.000);
    mm(1250, 4, 359491225496.000);
    mm(1500, 3, 685019722065.000);
    mm(1750, 2, 1001806087176.000);
    mm(2000, 2, 1974958963390.000);
    mm(2500, 1, 3060585851209.000);
    mm(3000, 1, 7694687665084.000);
    mm(3500, 1, 16754058063962.000);
    mm(4000, 1, 32845267978538.000);
}
